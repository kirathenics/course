# course
Библиотека opencv 4.7.0
Ссылка на скачивание: https://opencv.org/releases/
Гайд, как установить и добавить в проект: https://www.youtube.com/watch?v=8G7z8VOVl1M&list=PLxRwOZIK8ZHFGjvTYW_v-jKjYD3aRy4Um&index=41&t=44s&ab_channel=prgmir
Скопировать файл opencv_world470d.dll в папку x64/Debug/ из папки {путь к opencv}\opencv\build\x64\vc16\bin

Библиотека mpi
Ссылка на скачивание: https://www.microsoft.com/en-us/download/details.aspx?id=57467
Гайд, как добавить в проект: https://www.youtube.com/watch?v=L3cBUdLSb-M&ab_channel=RavellaAbhinav
Пути:
C/C++ Общие Дополнительные каталоги включаемых файлов:	$(MSMPI_INC);$(MSMPI_INC)\x64
Компоновщик Все параметры Дополнительные зависимости: msmpi.lib
Компоновщик Все параметры Дополнительные каталоги библиотек: $(MSMPI_LIB64)
