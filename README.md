zgui-qt
=======
Qt-интерфейс для видео-микшера.
За работу с видео\аудио отвечает сторонняя закрытая библиотека zcore.lib (см. папку lib).
Интерфейс описан в IManager.h, доступ к ней идёт через глобальный объект global_manager.


СБОРКА (Win)
=============
требуется:
- gstreamer-sdk (скачка http://docs.gstreamer.com/display/GstSDK/Installing+on+Windows, нужны Runtime & Development files )
- возможно WinSDK (нужна либа strmiids.lib)


ПРОЧЕЕ
=======
- интерфейс делается EN, с обязательной обёрткой для интернационализации.

