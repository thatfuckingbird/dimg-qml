#include <dimg/shims/include/kconfiggroup.h>
#include <QDebug>

KConfigGroup KConfigGroup::group(const QString &group) {
    KConfigGroup g;
    g.name = name + QStringLiteral(".") + group;
    return g;
}

KConfigGroup KSharedConfig::group(const QString &group) {
    KConfigGroup g;
    g.name = group;
    return g;
}

QVariant KConfigGroup::readEntry(const QString& key) const {
    qDebug() << "Configuration key requested:" << qUtf8Printable(name) << qUtf8Printable(key);
    return {};
}

void KConfigGroup::writeEntry(const QString& key, const QVariant&) {
    qDebug() << "Configuration key written:" << qUtf8Printable(name) << qUtf8Printable(key);
}

bool KConfigGroup::sync() {
    qDebug() << "Configuration sync requested";
    return true;
}
