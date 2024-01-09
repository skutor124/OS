Требуется подготовить модуль для загрузки в ядро Linux. Модуль должен:

сопровождаться Makefile для сборки;
оформляться в виде файла *.ko;
загружаться командой insmod и выгружаться командой rmmod;
при загрузке выводить в dmesg строку «Welcome to the Tomsk State University»;
при выгрузке выводить в dmesg строку «Tomsk State University forever!».


1) сначала создаём файл hello_TSU.c
2)Далее создаём Makefile и выполняем команду make, после чего появится файл модуля hello_TSU.ko
3)Загружаем модуль с помощью insmod: sudo insmod hello_TSU.ko
4)проверяем вывод с помощью sudo dmesg и видим: [ 1732.012556] Welcome to the Tomsk State University
5)Выгружаем модуль с помощью rmmod: sudo rmmod hello_TSU.ko
6)Проверяем вывод с помощью sudo dmesg и видим: [ 1887.678193] Tomsk State University forever!
