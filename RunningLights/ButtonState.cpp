#include "ButtonState.h"
// Получаем индекс массива по пину.
byte ButtonState::getIndex(byte Pin){

  byte index = 0;

  for ( byte i = 0; i < 10; i++){

    if ( Pin == state[i].Pin) {
       index = i;
    }

  }

  return index;

}
// Добавляем состояние для кнопки.
void ButtonState::Add(byte Pin, ButtonType buttonType){

  PressNum once = PressNum::One;
  // Кнопка с автоматическим срабатыванием после удержания в нажатом положении.
  if ( buttonType == ButtonType::Pressed ) { 
     once = PressNum::Many;
  }

  state[index] = { Pin, 0, Delay::NormalDelay, PressingState::NoPress, once, HeldDown::No};

  index++;

} 
// Проверяем состояние кнопки, подключенной к заданному пину.
bool ButtonState::CheckState(byte Pin){

  bool b = false;
  byte index = getIndex(Pin);
  // Получаем текущие состояние кнопки. 
  State s = state[index];
  // Получаем значение на входе контроллера
  // Кнопка нажата.
  if ( !digitalRead(Pin) ) { 
    // Увеличиваем счетчик нажатий кнопки.
    s.Count++;
    // Счетчик нажатий достиг значения, при котором считаем, что кнопка нажата. 
    if ( s.Count == s.Ready ) {
      // Срабатывает один раз при нажатии и удержание на кнопку.
      if ( s.Once == PressNum::One ) { 
        // Кнопка не была нажата.
        if ( s.OncePressing == PressingState::NoPress ) {
        // Меняем состояние кнопки на "нажата", чтобы не было повторных срабатываний
        s.OncePressing = PressingState::Pressing;
        b = true;
        }
      // Срабатывает пока удерживается кнопка с заданным интервалом.   
      } else {
        // Срабатывает, если уже нажата в течение PressDelay
         if ( s.HeldDown == HeldDown::Yes ) { 
          b = true;
          // Уменьшаем интервал счета. Начинаются автоматические срабатывания.
          s.Ready = Delay::AutoPress;
         } else {
          // Увеличиваем интервал счета. После запускается автоматическое срабатывание кнопки.
          s.Ready = Delay::PressDelay;
          // Меняем состояние кнопки на "удерживается в нажатом положение".
          s.HeldDown = HeldDown::Yes;  
         }
      } 
      // Сбрасываем счетчик нажатий.
      s.Count = 0;
      // Меняем состояние кнопки на "удерживается в нажатом положение".
      s.HeldDown = HeldDown::Yes;
    }
    // Сбрасываем счетчик подтверждения нажатия кнопки при достижении 170, чтобы не было ошибки.
    if (s.Count > 170) {
      s.Count = 0;
    }
  // Кнопка не нажата.
  } else {
    // Если кнопка была нажата.
    if ( s.OncePressing == PressingState::Pressing ) { 
      // Сбрасываем счетчик подтверждения нажатия.
      s.Count = 0;
      // Сбрасываем флаг "Нажата"
      s.OncePressing = PressingState::NoPress;
      // Сбрасываем флаг "Удерживается в нажатом положение"
      s.HeldDown = HeldDown::No;
    }

    if ( s.HeldDown == HeldDown::Yes ) {
      // Кнопка была нажата 1 раз в течение от 2 до 14 тактов. 
      if ( s.Count >= 2 && s.Count < 14 ) {
         b = true;
      }  
      // Сбрасываем счетчик подтверждения нажатия.
      s.Count = 0;
      // Сбрасываем флаг "Удерживается в нажатом положение"
      s.HeldDown = HeldDown::No;
      // Задаём снова интервал ожидания по умолчанию.
      s.Ready = Delay::NormalDelay;
    }

  }
  // Запоминаем состояние.
  state[index] = s;

  return b;

}
