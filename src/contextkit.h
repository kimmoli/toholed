#include <contextproperty.h>
#include <QCoreApplication>
#include <QTimer>
#include <QDate>
#include <QVariant>
#include <QDebug>

class Test : public QObject
{
    Q_OBJECT

public:

    Test(QObject *parent = 0) : QObject(parent)
        , time_(new ContextProperty("Time.SecondsSinceEpoch", this))
        , date_(new ContextProperty("Date.Current", this))
        {
        }

    void run()
    {
        connect(time_, SIGNAL(valueChanged()), this, SLOT(onTime()));
        connect(date_, SIGNAL(valueChanged()), this, SLOT(onDate()));
    }

private slots:

    void onTime()
    {
        qDebug() << time_->value() << " seconds is passed";
    }

    void onDate()
    {
        qDebug() << "Day is changed to " << date_->value();
    }
private:

    ContextProperty *time_;
    ContextProperty *date_;

};
