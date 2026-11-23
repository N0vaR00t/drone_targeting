4. No issues found
5.
   1. Звичайний запуск друкував середні значення, а помилка проявлялася під час заповнення та зсуву буфера.
      
      Інструмент - AddressSanitizer, Valgrind Memcheck.

      Фрагмент звіту:
        ```text
        ERROR: AddressSanitizer: heap-buffer-overflow
        READ of size 8
        SampleBuffer::push(double) (sample_buffer.cpp:15)
        ```

      Причина - цикл зсуву читав `data_[size_]`, елемент за виділеним буфером, що привело до виходу за межі heap-пам'яті та невизначеної поведінки.
      
      Виправлення - зсувати елементи від індексу `1` до `size_ - 1` у попередню позицію та не читати поза масивом.
      
   2. Програма друкувала неправильні суми `205032704` і середнє замість повних значень `15771746`.

      Інструмент - UndefinedBehaviorSanitizer. Valgrind Memcheck цей дефект не виявив.

      Фрагмент звіту:
        ```text
        budget.cpp:6:15: runtime error: signed integer overflow:
        1800000000 + 450000000 cannot be represented in type 'int'
        ```
      Причина - добуток і накопичена сума зберігалися в `int`, хоча правильна сума дорівнює `4,500,000,000` і перевищує `INT_MAX`.

      Виправлення - коефіцієнт, сума та типи повернення змінено на `long long`; середнє також виконується в цьому типі.

    3. Запуск друкував  `0.0 м/с` і «скид дозволено», але значення та рішення залежали від неініціалізованої пам'яті.

       Інструмент - Valgrind Memcheck. AddressSanitizer та UndefinedBehaviorSanitizer дефекту не виявили.

       Фрагмент звіту:
          ```text
          Conditional jump or move depends on uninitialised value(s)
          Uninitialised value was created by a heap allocation
              at load_profile (wind.cpp:6)
          ```
          Причина - `load_profile()` задавав `x` та `y`, але поле `gust_mps` залишалося неініціалізованим та друкувалося й використовувалось в умові `release_mode()`.

          Виправлення - `gust_mps` початкове значення `0.0`.
