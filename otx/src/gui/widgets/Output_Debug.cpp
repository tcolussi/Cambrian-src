
#include "Output_Debug.h"
#include <iostream>
#include <QString>
#include <QDateTime>

using namespace std;
Output_Debug::Output_Debug(QString Text,QString Title)
{
   QString date=QDateTime::currentDateTime().toString();

    cout << "Output Date:"+  date.toStdString()+ " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
    cout << Title.toStdString()+" : "+Text.toStdString()+"\n";
    cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n";
}

Output_Debug::Output_Debug(std::string Text, std::string Title, int Header)
{
   QString date=QDateTime::currentDateTime().toString();
   if (Header==1){
    cout << ">>>>>>>>>>>>>>>> Date:"+  date.toStdString()+ " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
   }
    cout << Title+" : "+Text+"\n";
   if (Header==2) {
    cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<Central Services<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n";
   }
}

Output_Debug::~Output_Debug()
{

}
