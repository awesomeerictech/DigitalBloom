// sqliteone.hpp — prefer unqualified columns unless caller supplies tableName
#pragma once
#include <QString>
#include <QMap>
#include <QVariant>
#include <QSet>
#include <QRegularExpression>

// Escape single quotes for SQL literals
static QString escapeSqlString(const QString &s) {
    QString out = s;
    out.replace("'", "''");
    return out;
}

static QString stripBrackets(const QString &s) {
    QString r = s.trimmed();
    if (r.startsWith('[') && r.endsWith(']') && r.length() > 1) {
        return r.mid(1, r.length() - 2).trimmed();
    }
    return r;
}

/*
 * Build a SQL WHERE clause (WITH leading "WHERE ") from params.
 *
 * - params: column -> value. If the value is null/empty -> skipped.
 * - numericKeys: set of column names that should be treated as numeric (case-insensitive).
 * - tableName: optional table/class name to qualify columns with; default is empty (no qualification).
 *
 * Returns empty string if no conditions; otherwise "WHERE <cond>".
 */
static QString buildWhereClause(const QMap<QString, QVariant> &params,
                                const QSet<QString> &numericKeys = QSet<QString>(),
                                const QString &tableName = QStringLiteral("cryptocenter"))
{
    QStringList parts;

    QSet<QString> numericLower;
    for (const QString &k : numericKeys) numericLower.insert(k.trimmed().toLower());

    static const QRegularExpression numberRe(R"(^[+-]?(?:\d+)(?:\.\d+)?(?:[eE][+-]?\d+)?$)");

    auto qualifyIfNeeded = [&](const QString &rawKey) -> QString {
        QString k = rawKey.trimmed();
        if (k.isEmpty()) return QString();

        // if already looks qualified (table.col or [table].[col]) -> bracket both sides
        if (k.contains('.')) {
            QString left = k.section('.', 0, 0).trimmed();
            QString right = k.section('.', 1).trimmed();
            left = stripBrackets(left);
            right = stripBrackets(right);
            return QString("[%1].[%2]").arg(left, right);
        }

        // if caller didn't request qualification, return the bare column (maybe bracketed)
        if (tableName.trimmed().isEmpty()) {
            // preserve any existing brackets around the column
            QString col = stripBrackets(k);
            return QString("[%1]").arg(col); // bracket the column only for safety
        }

        // otherwise qualify with provided table name
        QString tbl = stripBrackets(tableName.trimmed());
        QString col = stripBrackets(k);
        return QString("[%1].[%2]").arg(tbl, col);
    };

    auto bareColumn = [&](const QString &rawKey) -> QString {
        QString k = rawKey.trimmed();
        if (k.contains('.')) k = k.section('.', -1);
        k = stripBrackets(k);
        return k.trimmed();
    };

    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        const QString rawKey = it.key().trimmed();
        QVariant val = it.value();
        if (rawKey.isEmpty()) continue;
        if (!val.isValid() || val.isNull()) continue;

        QString sval;
        switch (val.type()) {
        case QMetaType::QString: sval = val.toString(); break;
        case QMetaType::QByteArray: sval = QString::fromUtf8(val.toByteArray()); break;
        case QMetaType::Int:
        case QMetaType::LongLong:
        case QMetaType::UInt:
        case QMetaType::ULongLong:
        case QMetaType::Double:
        case QMetaType::Float:
        case QMetaType::Short:
        case QMetaType::Long:
            sval = val.toString(); break;
        case QMetaType::Bool: sval = val.toBool() ? QStringLiteral("1") : QStringLiteral("0"); break;
        default: sval = val.toString();
        }

        if (sval.trimmed().isEmpty()) continue;
        QString trimmedVal = sval.trimmed();

        QString bare = bareColumn(rawKey).toLower();
        bool keyMarkedNumeric = numericLower.contains(bare);
        bool valueLooksNumeric = numberRe.match(trimmedVal).hasMatch();

        const QString qualifiedKey = qualifyIfNeeded(rawKey);

        if (keyMarkedNumeric || valueLooksNumeric) {
            parts << QString("%1 = %2").arg(qualifiedKey, trimmedVal);
        } else {
            parts << QString("%1 = '%2'").arg(qualifiedKey, escapeSqlString(trimmedVal));
        }
    }

    if (parts.isEmpty()) return QString();
    return QString("WHERE %1").arg(parts.join(" AND "));
}


static QString buildWhereClauseBetter(const QMap<QString, QVariant> &params,
                                const QSet<QString> &numericKeys = QSet<QString>(),
                                const QString &tableName = QStringLiteral("cryptocenter"))
{
    QStringList parts;

    // Normalize numericKeys to lowercase for case-insensitive matching
    QSet<QString> numericLower;
    for (const QString &k : numericKeys) {
        numericLower.insert(k.trimmed().toLower());
    }

    // simple numeric regex: optional sign, digits, optional decimal, optional exponent
    static const QRegularExpression numberRe(R"(^[+-]?(?:\d+)(?:\.\d+)?(?:[eE][+-]?\d+)?$)");

    auto qualify = [&](const QString &rawKey) -> QString {
        QString k = rawKey.trimmed();
        if (k.isEmpty()) return QString();

        // If already contains a dot, split and bracket both parts
        if (k.contains('.')) {
            QString left = k.section('.', 0, 0).trimmed();
            QString right = k.section('.', 1).trimmed();
            left = stripBrackets(left);
            right = stripBrackets(right);
            return QString("[%1].[%2]").arg(left, right);
        }

        // Single name -> bracket column and table
        QString tbl = tableName.trimmed();
        tbl = stripBrackets(tbl);
        QString col = stripBrackets(k);
        return QString("[%1].[%2]").arg(tbl, col);
    };

    auto bareColumn = [&](const QString &rawKey) -> QString {
        QString k = rawKey.trimmed();
        if (k.contains('.')) k = k.section('.', -1);
        k = stripBrackets(k);
        return k.trimmed();
    };

    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        const QString rawKey = it.key().trimmed();
        QVariant val = it.value();

        if (rawKey.isEmpty()) continue;
        if (!val.isValid() || val.isNull()) continue;

        // Convert many types to string for uniform handling
        QString sval;
        switch (val.type()) {
        case QMetaType::QString:
            sval = val.toString();
            break;
        case QMetaType::QByteArray:
            sval = QString::fromUtf8(val.toByteArray());
            break;
        case QMetaType::Int:
        case QMetaType::LongLong:
        case QMetaType::UInt:
        case QMetaType::ULongLong:
        case QMetaType::Double:
        case QMetaType::Float:
        case QMetaType::Short:
        case QMetaType::Long:
            sval = val.toString();
            break;
        case QMetaType::Bool:
            sval = val.toBool() ? QStringLiteral("1") : QStringLiteral("0");
            break;
        default:
            sval = val.toString();
        }

        if (sval.trimmed().isEmpty()) continue;
        QString trimmedVal = sval.trimmed();

        QString bare = bareColumn(rawKey).toLower();
        bool keyMarkedNumeric = numericLower.contains(bare);
        bool valueLooksNumeric = numberRe.match(trimmedVal).hasMatch();

        const QString qualifiedKey = qualify(rawKey);

        if (keyMarkedNumeric || valueLooksNumeric) {
            parts << QString("%1 = %2").arg(qualifiedKey, trimmedVal);
        } else {
            parts << QString("%1 = '%2'").arg(qualifiedKey, escapeSqlString(trimmedVal));
        }
    }

    if (parts.isEmpty()) return QString();
    return QString("WHERE %1").arg(parts.join(" AND "));
}

static QString buildWhereClauseOne(const QMap<QString, QVariant> &params,
                                const QSet<QString> &numericKeys = QSet<QString>())
{
    QStringList parts;

    // Normalize numericKeys to lowercase for case-insensitive matching
    QSet<QString> numericLower;
    for (const QString &k : numericKeys) {
        numericLower.insert(k.trimmed().toLower());
    }

    // simple numeric regex: optional sign, digits, optional decimal, optional exponent
    static const QRegularExpression numberRe(R"(^[+-]?(?:\d+)(?:\.\d+)?(?:[eE][+-]?\d+)?$)");

    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        const QString key = it.key().trimmed();
        QVariant val = it.value();

        // ignore empty/invalid column names
        if (key.isEmpty()) continue;

        // treat invalid/null variant as absent
        if (!val.isValid() || val.isNull()) continue;

        // Convert many types to string for uniform handling
        QString sval;
        switch (val.type()) {
        case QMetaType::QString:
            sval = val.toString();
            break;
        case QMetaType::QByteArray:
            sval = QString::fromUtf8(val.toByteArray());
            break;
        case QMetaType::Int:
        case QMetaType::LongLong:
        case QMetaType::UInt:
        case QMetaType::ULongLong:
        case QMetaType::Double:
        case QMetaType::Float:
        case QMetaType::Short:
        case QMetaType::Long:
            // Use toString for numeric types
            sval = val.toString();
            break;
        case QMetaType::Bool:
            sval = val.toBool() ? QStringLiteral("1") : QStringLiteral("0");
            break;
        default:
            // fallback to toString() (covers QVariant holding QStringList, etc.)
            sval = val.toString();
        }

        // Trim and drop empty strings
        if (sval.trimmed().isEmpty()) continue;
        QString trimmed = sval.trimmed();

        // Decide if this key should be numeric
        bool keyMarkedNumeric = numericLower.contains(key.toLower());
        bool valueLooksNumeric = numberRe.match(trimmed).hasMatch();

        if (keyMarkedNumeric || valueLooksNumeric) {
            // Use the trimmed numeric literal directly (no quotes)
            parts << QString("%1 = %2").arg(key, trimmed);
        } else {
            // Quote & escape string
            parts << QString("%1 = '%2'").arg(key, escapeSqlString(trimmed));
        }
    }

    if (parts.isEmpty()) {
        return QString(); // no WHERE clause
    }

    // Return the full WHERE clause expected by your QxOrm adapter
    return QString("WHERE %1").arg(parts.join(" AND "));
}






static bool isAllowedColumn(const QString &col) {
    static const QSet<QString> allowed = {
        "username","token","name","email","iv","cipher","key",
        // add all allowed column names here
    };
    return allowed.contains(col);
}

/*

########## Username only #######################

QMap<QString,QVariant> p;
p["username"] = myusername;
QString where = buildWhereClause(p); // "username = 'bob'"
modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where);

######### Username + token (token numeric) #####

QMap<QString,QVariant> p;
p["username"] = myusername;
p["token"] = token1; // integer
QString where = buildWhereClause(p, QSet<QString>() << "token");
modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where);

######### Username + token + hint (hint optional) ##########

QMap<QString,QVariant> p;
p["username"] = myusername;
p["token"] = token1;
p["hint"] = myhint; // if empty, builder will skip it
QString where = buildWhereClause(p, QSet<QString>() << "token");
modifyMyJsonToolsuser->modifyJsonValue(userdoc, "query.sql", where);

######### Generic usage with param1/param2/... (you can fill keys dynamically) ##########

QMap<QString,QVariant> params;
params["param1"] = value1;
params["param2"] = value2;
params["param3"] = value3;
QStringList numeric = {"param2"}; // mark whichever are numeric
QString where = buildWhereClause(params, QSet<QString>::fromList(numeric));



*/
