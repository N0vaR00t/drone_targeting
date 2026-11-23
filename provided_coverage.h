// provided_coverage.h — надано лектором (ДЗ 8).
//
// Прискорена перевірка накриття. Замість обчислення відстані (std::sqrt на кожну пару
// «ціль × ціль») об'єкт один раз готує таблицю КВАДРАТІВ радіусів ураження, після чого
// кожна перевірка коштує лише множення й порівняння. Результат перевірки збігається зі
// звичайним порівнянням відстані з радіусом: обидві величини невід'ємні, тож піднесення
// до квадрата порядку не змінює.
//
// Модуль передано з суміжного проєкту; діагностичні збірки на ньому не запускалися.
//
// Інтеграція: створити об'єкт один раз перед перебором кандидатів, передавши радіуси
// боєприпасів у тому самому порядку, у якому вони лежать у таблиці боєприпасів; далі
// замість порівняння відстані з радіусом викликати covers().
#ifndef PROVIDED_COVERAGE_H
#define PROVIDED_COVERAGE_H

#include <cstddef>

class CoverageChecker {
public:
    // Максимальна кількість типів боєприпасів, яку підтримує таблиця.
    static const std::size_t MAX_AMMO_KINDS = 3;

    // radii — радіуси ураження, kinds — скільки їх (не більше MAX_AMMO_KINDS).
    CoverageChecker(const double* radii, std::size_t kinds) : kinds_(kinds) {
        for (std::size_t i = 0; i < kinds_; ++i) {
            r2_[i] = radii[i] * radii[i];
        }
    }

    // dx, dy — різниця координат між точкою влучання та ціллю.
    bool covers(std::size_t ammo_index, double dx, double dy) const {
        return dx * dx + dy * dy <= r2_[ammo_index];
    }

    std::size_t kinds() const { return kinds_; }

private:
    std::size_t kinds_;
    double r2_[MAX_AMMO_KINDS];
};

#endif  // PROVIDED_COVERAGE_H
