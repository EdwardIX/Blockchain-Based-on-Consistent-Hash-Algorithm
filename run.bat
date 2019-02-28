@echo off
copy User.exe ..\tmp\
copy Checker.exe ..\tmp\
start cmd /c "God&pause"
start cmd /c "Server&pause"
start cmd /c "User&pause"
start cmd /c "..\tmp\User&pause"