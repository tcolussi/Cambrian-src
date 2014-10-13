#ifndef OUTPUT_DEBUG_H
#define OUTPUT_DEBUG_H
#include <QString>
class Output_Debug
{

public:
    explicit Output_Debug(QString Text="",QString Title="");
    explicit Output_Debug(std::string Text,std::string Title);
     explicit Output_Debug(std::string Text,std::string Title,int Header);
    ~Output_Debug();


};


#endif // OUTPUT_DEBUG_H
