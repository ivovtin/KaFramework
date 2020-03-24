Программная среда для анализа событий с детектора КЕДР и набор утилит. <br />

Описание файлов пакета:                                         <br />
kframework.cc           Главный модуль и API библиотеки анализа <br />
kframework.h                                                    <br />

kf_event_stat.cc	Универсальный сборщик статистики событий  <br />
kf_event_stat.h		разного типа с печатью                    <br />

mknatfilelist.cc	Утилита для формирования списка nat-файлов <br />
mknatfilelist.h         из списка аргументов, которыми могут быть:
			имя nat-файла,номер захода,интервал номеров заходов,
			файл со списком любого из вышеперечисленного

Makefile                make-файл, использует KcReleaseTools/rules.mk  <br />

eventbr.h		Вспомогательные файлы для создания дерева      <br />
vddcbr.h		с реконструированной информацией в Rootе       <br />
vddcbr.cc                                                              <br />
emcbr.h                                                                <br />
emcbr.cc                                                               <br />
atcbr.h                                                                <br />
atcbr.cc                                                               <br />
tofbr.h                                                                <br />
tofbr.cc                                                               <br />
mubr.h                                                                 <br />
mubr.cc                                                                <br />
runtypes.h              Идентификаторы существующих типов заходов      <br />

