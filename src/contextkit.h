#include <contextproperty.h>
#include <QDebug>

class Test : public QObject
{
    Q_OBJECT

public:

    Test(QObject *parent = 0) : QObject(parent)
        , alarm_(new ContextProperty("Alarm.Present", this))
        {}

    void run()
    {
        connect(alarm_, SIGNAL(valueChanged()), this, SLOT(onAlarm()));
    }

private slots:

    void onAlarm()
    {
        qDebug() << alarm_->value();
    }

private:

    ContextProperty *alarm_;

};
