#ifndef NAVEGATION_H
#define NAVEGATION_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Navegation;
}
QT_END_NAMESPACE

class Navegation : public QMainWindow
{
    Q_OBJECT

public:
    explicit Navegation(QWidget *parent = 0);
    ~Navegation();

signals:
    void display(int number);

private slots:
    void on_ModbusClientButton_clicked();
    void on_ModbusServerButton_clicked();

private slots:
    void initForm();
    void initStyle();

private:
    Ui::Navegation * ui;
};

#endif // NAVEGATION_H
