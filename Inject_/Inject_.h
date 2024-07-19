#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Inject_.h"

class Inject_ : public QMainWindow
{
    Q_OBJECT

public:
    Inject_(QWidget *parent = Q_NULLPTR);
    ~Inject_();

private:
    Ui::Inject_Class ui;
};
