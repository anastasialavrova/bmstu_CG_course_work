#ifndef RENDERUTILITIES_H
#define RENDERUTILITIES_H

// Регулировка цвета путем умножения на некоторое соотношение
// Возвращает: 32-битное значение ARGB, каждый канал изменяется согласно заданному соотношению
unsigned int multiplyColorChannels(unsigned int color, double ratio);

// Регулировка цвета на канал путем умножения на набор соотношений каналов
unsigned int multiplyColorChannels(unsigned int color, double alphaRatio, double redRatio, double greenRatio, double blueRatio);

// Умножаем цвет на набор упакованных цветовых каналов
unsigned int multiplyColorChannels(unsigned int color, unsigned int intensities);

// Добавить 2 цвета вместе (без переполнения)
// Возвращает: беззнаковое целое из двух цветов, добавленных вместе, канал за каналом
unsigned int addColors(unsigned int color1, unsigned int color2);

// Получить случайный цвет ARGB
// Возвращает: целое число без знака, содержащее цвет ARGB, с A = FF / 100%
unsigned int getRandomColor();

// Объединяем цветные каналы в один беззнаковый int
unsigned int combineColorChannels(double red, double green, double blue);

// Извлекаем цветовой канал как двойной в [0, 1]
// Флаги канала: 0 = альфа, 1 = красный, 2 = зеленый, 3 = синий
double extractColorChannel(unsigned int color, int channel);

// Рассчитать перспективную правильную линейную интерполяцию некоторого значения
double getPerspCorrectLerpValue(double startVal, double startZ, double endVal, double endZ, double ratio);

#endif // RENDERUTILITIES_H
