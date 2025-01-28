class Sensor
{
public:
    Sensor(std::string name) : name(name) {}

    /**
     * Увеличивает значение сенсора на указанное значение
     */
    void inc(int value)
    {
        this->value += value;
    }

    /**
     * Сбрасывает значение счетчика в 0
     */
    void reset()
    {
        this->value = 0;
    }

private:
    // Имя сенсора
    std::string name;

    // Значение сенсора
    int value = 0;
};
