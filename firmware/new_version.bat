set /p version=<version.txt
set /A version=version+1
>version.txt echo %version%
copy ..\Debug\sketch_may14a.bin new.bin
rename new.bin firmware_%version%.bin