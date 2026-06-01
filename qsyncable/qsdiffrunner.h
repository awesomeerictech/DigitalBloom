

#ifndef QSDIFFRUNNER_H
#define QSDIFFRUNNER_H

#include "qspatch.h"
#include "qspatchable.h"
#include "qsyncablefunctions.h"

class QSDiffRunner
{
public:
    QSDiffRunner();

    QString keyField() const;

    void setKeyField(const QString &value);

    QSPatchSet compare(const QVariantList& from,
                       const QVariantList& to);

    bool patch(QSPatchable* patchable, const QSPatchSet& patches) const;

signals:

public slots:

private:


    QString m_keyField;
};

#endif // QSDIFFRUNNER_H
