#ifndef STATEFSPROPERTY_H
#define STATEFSPROPERTY_H

#include <QObject>
#include <contextproperty.h>

class StatefsProperty : public QObject
{
    Q_OBJECT
public:
    explicit StatefsProperty(QObject *parent = 0);

    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)
    QString key() const;
    void setKey(const QString &newKey);

    Q_PROPERTY(QVariant value READ value NOTIFY valueChanged)
    QVariant value() const;

    QScopedPointer<ContextProperty> property;

signals:
    void keyChanged();
    void valueChanged(QVariant value);

private slots:
    void onValueChanged();
};

#endif // STATEFSPROPERTY_H
