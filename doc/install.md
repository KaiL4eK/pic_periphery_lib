## Установка библиотеки

Допустим, вы создали проект, примерный вид дерева проекта будет иметь следующий вид:

![alt text][src_state]

Первым шагом необходимо добавить файлы с исходным кодом (*source files*) в ваш проект
Необходимо нажать правой кнопкой на папку `Source Files` и выбрать пункт `Add Existing Items from Folders...`

![alt text][install1]

Далее нажимаете `Add folder` и указываете путь до папки с библиотекой

![alt text][install2]

В результате в дереве проекта должны появиться исходные файлы библиотеки

![alt text][install3]

Далее необходимо указать путь поиска заголовочных файлов (*header files*)
Необходимо зайти в свойства проекта
*Нажимаем правой кнопкой по проекту, в самом низу выбираем `Properties`*

Выбираем категорию `XC16 (Global Options)` -> `Common include dirs` -> `...`
Далее жмем кнопку `Browse` и выбираем путь до папки

![alt text][install4]

В результате относительный путь должен появиться в перечислении путей:

![alt text][install5]

[src_state]: https://github.com/KaiL4eK/pic_periphery_lib/blob/master/doc/img/install1.png "Initial state"
[install1]: https://github.com/KaiL4eK/pic_periphery_lib/blob/master/doc/img/install2.png "First step"
[install2]: https://github.com/KaiL4eK/pic_periphery_lib/blob/master/doc/img/install3.png "Next step"
[install3]: https://github.com/KaiL4eK/pic_periphery_lib/blob/master/doc/img/install4.png "Next step"
[install4]: https://github.com/KaiL4eK/pic_periphery_lib/blob/master/doc/img/install5.png "Next step"
[install5]: https://github.com/KaiL4eK/pic_periphery_lib/blob/master/doc/img/install6.png "Next step"

После выполнения данных шагов можно использовать библиотеку периферии
