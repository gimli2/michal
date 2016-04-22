@echo off
cd %CD%\pictures_data
dir /s/b > .\lists\file_names.txt

cd %CD%\..
.\src\main.exe -d 0 -i .\pictures_data\lists\file_names.txt -t .\out\pictures_data_all_out.txt