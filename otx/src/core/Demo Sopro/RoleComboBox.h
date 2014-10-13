#ifndef ROLECOMBOBOX_H
#define ROLECOMBOBOX_H

#include <QtGui>
#include <QMessageBox>
#include <QComboBox>
#include <../opentxs/OTAPI.hpp>
#include "OTX.hpp"
#include <../opentxs/OTAPI_Exec.hpp>

//Derived Class from QComboBox

class RoleComboBox: public QComboBox
{
  Q_OBJECT
  public:
    RoleComboBox(QWidget* parent):QComboBox(parent)
    {
      this->setParent(parent);
      connect(this , SIGNAL(currentIndexChanged(int)),this,SLOT(handleSelectionChanged(int)));

    };
    ~RoleComboBox(){};

  public slots:
    //Slot that is called when QComboBox selection is changed
    void handleSelectionChanged(int index)
    {

        if (currentText()=="<Create New Role..>" and count() > 1){
        OTX::It()->mc_defaultnym_slot();
        }
        else
        {
            std::string current_nym = OTAPI_Wrap::It()->GetNym_ID(index);
            OTX::It()->mc_show_nym_slot(QString::fromUtf8(current_nym.c_str()));

            //QMessageBox* msg = new QMessageBox();
            //msg->setWindowTitle("Credential Window");
           // std::string text="nym:"+ currentText().toStdString()+ " nym id:" + OTAPI_Wrap::It()->GetNym_ID(index);
           // msg->setText(QString::fromUtf8(text.c_str()));

            //msg->show();

        }

    };



};



#endif // ROLECOMBOBOX_H
