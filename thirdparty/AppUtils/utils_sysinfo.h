#ifndef UTILS_SYSINFO_H
#define UTILS_SYSINFO_H
/* ************************************************************************** */

#include <QObject>
#include <QVariantMap>

/* ************************************************************************** */

/*!
 * \brief The UtilsSysInfo class
 */
class UtilsSysInfo: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString cpu_arch READ getCpuArch CONSTANT)
    Q_PROPERTY(int cpu_coreCount_physical READ getCpuCoreCountPhysical CONSTANT)
    Q_PROPERTY(int cpu_coreCount_logical READ getCpuCoreCountLogical CONSTANT)
    Q_PROPERTY(quint64 ram_total READ getRamTotal CONSTANT)

    Q_PROPERTY(QString os_name READ getOsName CONSTANT)
    Q_PROPERTY(QString os_version READ getOsVersion CONSTANT)
    Q_PROPERTY(QString os_display_server READ getOsDisplayServer CONSTANT)

    QString m_cpu_arch;
    int m_cpu_core_physical = 0;
    int m_cpu_core_logical = 0;

    uint64_t m_ram_total = 0;

    QString m_os_name;
    QString m_os_version;
    QString m_os_displayserver;

    // Singleton
    static UtilsSysInfo *instance;
    UtilsSysInfo();
    ~UtilsSysInfo();

    void getCpuInfos();
    void getRamInfos();

public:
    static UtilsSysInfo *getInstance();

    void printInfos();

    Q_INVOKABLE QString getCpuArch() const { return m_cpu_arch; }

    Q_INVOKABLE int getCpuCoreCountPhysical() const { return m_cpu_core_physical; }

    Q_INVOKABLE int getCpuCoreCountLogical() const { return m_cpu_core_logical; }

    Q_INVOKABLE uint64_t getRamTotal() const  { return m_ram_total; }

    Q_INVOKABLE QString getOsName() const { return m_os_name; }

    Q_INVOKABLE QString getOsVersion() const { return m_os_version; }

    Q_INVOKABLE QString getOsDisplayServer() const { return m_os_displayserver; }
};

/* ************************************************************************** */
#endif // UTILS_SYSINFO_H
