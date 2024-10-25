
#include <Arduino.h>

// Типы кнопок. Normal - обычная. Pressed - с автоматическим включением.
enum ButtonType { Normal = 1, Pressed = 2};
// Состояние кнопки. NoPress - не нажата. Pressing - нажата.
enum PressingState { NoPress = 1, Pressing = 2};
// Сколько раз может быть нажата кнопка. One - один. Many - много.
enum PressNum {One, Many};
// Кнопка удерживается
enum HeldDown {Yes, No};
// Задержки для нажатых кнопок
// NormalDelay - нормальнонажатая кнопка
// PressDalay - задержка для начала автоматического нажатия
// AutoPress - задержка между нажатиями в автоматическом режиме при удержание кнопки нажатой.  
enum Delay {NormalDelay = 2, PressDelay = 20, AutoPress = 6};

// Создаём структуру для хранения состояния кнопки.
struct State {
  byte Pin;                    // Номер пина, к которому подключена кнопка.
  byte Count;                  // Счетчик нажатий
  Delay Ready;                 // Срабатывает при достижении числа нажатий
  PressingState OncePressing;  // Нажатие один раз. Кнопка была нажата или нет.
  PressNum Once;               // Кнопка нажимается один раз или начинают срабатывать нажатия после удержания в нажатом положении. 
  HeldDown HeldDown;           // Кнопка удерживается нажатой.
};
// Создаём класс для хранения состояний кнопок. 
class ButtonState {

  private:

    byte index;
    // 10 кнопок.
    State state[10];
    bool debug;

    byte getIndex(byte Pin);

  public:

    ButtonState(){

       index = 0;
       debug = true; 
        
    }
    // Добавляем кнопку.
    void Add(byte Pin, ButtonType buttonType);
    // Проверяем состояние кнопки.
    bool CheckState(byte Pin);
    
};