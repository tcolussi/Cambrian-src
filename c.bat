@Echo Off
Echo Creating backup of source files

IF "%1" == "" GOTO ErrorMissingFolder

Echo Copying Project Files (*.cpp, *.h, *.pro*) into folder %1

Copy *.cpp %1
Copy *.h %1
Copy *.pro %1
Copy *.txt %1

Goto Done

:ErrorMissingFolder
Echo Missing Backup Folder

:Done
