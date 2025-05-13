#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <limits>

using namespace std;

// Enum for Element Types
enum class ElementType {
    Fire, Water, Earth, Air, None
};

// Helper to get string from ElementType
string elementToString(ElementType type) {
    switch (type) {
    case ElementType::Fire: return "Fire";
    case ElementType::Water: return "Water";
    case ElementType::Earth: return "Earth";
    case ElementType::Air: return "Air";
    default: return "None";
    }
}

// Elemental weaknesses
float getElementalMultiplier(ElementType attacker, ElementType defender) {
    if (attacker == ElementType::Fire && defender == ElementType::Earth) return 1.5;
    if (attacker == ElementType::Water && defender == ElementType::Fire) return 1.5;
    if (attacker == ElementType::Earth && defender == ElementType::Air) return 1.5;
    if (attacker == ElementType::Air && defender == ElementType::Water) return 1.5;

    if (attacker == ElementType::Earth && defender == ElementType::Fire) return 0.5;
    if (attacker == ElementType::Fire && defender == ElementType::Water) return 0.5;
    if (attacker == ElementType::Water && defender == ElementType::Air) return 0.5;
    if (attacker == ElementType::Air && defender == ElementType::Earth) return 0.5;

    return 1.0;
}

// Base Character Class
class Character {
protected:
    string name;
    int health;
    int defense;
    int strength;
    int mana;
    int level;
    int speed;
    int experience;
    int maxHealth;
    int maxMana;
    ElementType type;

public:
    Character(string name, int hp, int def, int str, int mana, int lvl, int spd, ElementType type)
        : name(name), health(hp), maxHealth(hp), defense(def), strength(str),
        mana(mana), maxMana(mana), level(lvl), speed(spd), type(type), experience(0) {}

    virtual ~Character() = default;

    virtual void takeDamage(int dmg) {
        int damageTaken = dmg - defense;
        damageTaken = max(0, damageTaken);
        health -= damageTaken;
        cout << name << " takes " << damageTaken << " damage! (HP: " << max(0, health) << ")\n";
    }

    void useMana(int amount) {
        mana = max(0, mana - amount);
    }

    bool isAlive() const {
        return health > 0;
    }

    int getHealth() const { return health; }

    string getName() const { return name; }
    int getSpeed() const { return speed; }
    ElementType getType() const { return type; }
    int getMana() const { return mana; }

    virtual void attack(Character& target) {
        cout << name << " uses Strength Attack on " << target.getName() << "!\n";
        target.takeDamage(strength);
    }

    virtual void castSpell(Character& target, ElementType spellType) {
        int manaCost = 20;
        if (mana < manaCost) {
            cout << name << " doesn't have enough mana!\n";
            return;
        }
        useMana(manaCost);
        float multiplier = getElementalMultiplier(spellType, target.getType());
        int spellDamage = static_cast<int>(strength * multiplier);
        cout << name << " casts a " << elementToString(spellType) << " spell on "
            << target.getName() << " for " << spellDamage << " damage!\n";
        target.takeDamage(spellDamage);

    }
};

// Player class
class Player : public Character {
private:
    int healingPotions = 7;
    int manaPotions = 5;

public:
    Player(string name, ElementType type)
        : Character(name, 200, 10, 20, 140, 1, 15, type) {}

    void castMultiSpell(vector<shared_ptr<Character>>& enemies, ElementType spellType) {
        const int manaCost = 35;
        if (mana < manaCost) {
            cout << name << " doesn't have enough mana to cast a multi-target spell!\n";
            return;
        }

        useMana(manaCost);

        string spellName;
        switch (spellType) {
        case ElementType::Fire: spellName = "Fireball"; break;
        case ElementType::Water: spellName = "Flood"; break;
        case ElementType::Earth: spellName = "Rock blast"; break;
        case ElementType::Air: spellName = "Tempest"; break;
        default: spellName = "Unknown Spell"; break;
        }

        cout << name << " casts " << spellName << " on all enemies!\n";

        for (auto& enemy : enemies) {
            if (!enemy->isAlive()) continue;

            float multiplier = getElementalMultiplier(spellType, enemy->getType());
            int damage = static_cast<int>(strength * multiplier * 0.75); // AoE is slightly weaker than single-target
            cout << " - Hits " << enemy->getName() << " for " << damage << " damage.\n";
            enemy->takeDamage(damage);
        }
    }


    void chooseAction(vector<shared_ptr<Character>>& enemies) {
        cout << "\n--- Player Status ---\n";
        cout << name << " - HP: " << health << "/" << maxHealth
            << ", MP: " << mana << "/" << maxMana << ", Level: " << level << "\n";
        cout << "----------------------\n";

        cout << "Choose an action:\n";
        cout << "1. Strength Attack\n2. Cast Spell\n3. Use Item\nChoice: ";
        
        int choice;
        cin >> choice;

        if (choice == 3) {
            useItem();
            return;
        }

        bool multiTargetUnlocked = level >= 3;

        ElementType spell = ElementType::None;
        int targetIndex = -1;

        if (choice == 2) {
            cout << "Choose spell type:\n1. Fireball\n2. Flood\n3. Quake\n4. Tempest\nChoice: ";
            int spellChoice;
            cin >> spellChoice;
            spell = static_cast<ElementType>(spellChoice - 1);
        }

        if (choice == 1 || (choice == 2 && !multiTargetUnlocked)) {
            cout << "Choose enemy to target:\n";
            for (size_t i = 0; i < enemies.size(); ++i) {
                if (enemies[i]->isAlive())
                    cout << i + 1 << ". " << enemies[i]->getName() << "\n";
            }
            cin >> targetIndex;
            if (targetIndex < 1 || targetIndex > enemies.size() || !enemies[targetIndex - 1]->isAlive()) {
                cout << "Invalid target.\n";
                return;
            }
        }

        if (choice == 1) {
            attack(*enemies[targetIndex - 1]);
        }
        else if (choice == 2) {
            if (multiTargetUnlocked) {
                castMultiSpell(enemies, spell);
            }
            else {
                castSpell(*enemies[targetIndex - 1], spell);
            }
        }
        else {
            cout << "Invalid action.\n";
        }
    }


    void healFull() {
        health = maxHealth;
        mana = maxMana;
    }

    void useItem() {
        cout << "Items:\n";
        cout << "1. Healing Potion (" << healingPotions << " left)\n";
        cout << "2. Mana Potion (" << manaPotions << " left)\n";
        cout << "Choose item: ";
        int item;
        cin >> item;
        if (item == 1 && healingPotions > 0) {
            int heal = 60;
            health = min(health + heal, maxHealth);
            healingPotions--;
            cout << "You used a Healing Potion! HP: " << health << "\n";
        }
        else if (item == 2 && manaPotions > 0) {
            int mp = 30;
            mana = min(mana + mp, maxMana);
            manaPotions--;
            cout << "You used a Mana Potion! MP: " << mana << "\n";
        }
        else {
            cout << "Invalid or no potions left!\n";
        }
    }

    void gainExperience(int exp) {
        experience += exp;
        cout << "You gained " << exp << " EXP!\n";
        while (experience >= 100) {
            levelUp();
            experience -= 100;
        }
    }

    void levelUp() {
        level++;
        maxHealth += 20;
        health = maxHealth;
        strength += 5;
        defense += 2;
        speed += 2;
        maxMana += 10;
        mana = maxMana;
        cout << "You leveled up! Level: " << level << "\n";
    }
};


// Enemy class
class Enemy : public Character {
public:
    int expReward = 50;

    Enemy(string name, ElementType type)
        : Character(name, 80, 5, 15, 50, 1, rand() % 10 + 5, type) {}

    virtual void takeTurn(shared_ptr<Player>& player) {
        if (mana >= 20 && rand() % 2 == 0) {
            ElementType spell = static_cast<ElementType>(rand() % 4);
            castSpell(*player, spell);
        }
        else {
            attack(*player);
        }
    }
};

class Crony : public Enemy {
public:
    Crony(string name, ElementType type)
        : Enemy(name, type) {
        health = maxHealth = 90;
        strength = 18;
        defense = 6;
        speed = 10;
        mana = maxMana = 40;
        expReward = 70;
    }
};

class FinalBoss : public Enemy {
public:
    FinalBoss()
        : Enemy("Dragon King", ElementType::Fire) {
        health = maxHealth = 150;
        strength = 35;
        defense = 15;
        speed = 20;
        mana = maxMana = 150;
        expReward = 100;
    }

    void takeTurn(shared_ptr<Player>& player) override {
        if (mana >= 30 && rand() % 2 == 0) {
            ElementType spell = ElementType::Fire;
            cout << getName() << " unleashes INFERNO!\n";
            player->takeDamage(static_cast<int>(strength * 1.2));
            useMana(30);
        }
        else {
            attack(*player);
        }
    }
};



// Game class
class Game {
private:
    shared_ptr<Player> player;
    vector<shared_ptr<Character>> enemies;

public:
    void setup() {
        string name;
        cout << "Enter your name: ";
        cin >> name;

        cout << "Choose your element:\n1. Fire\n2. Water\n3. Earth\n4. Air\nChoice: ";
        int choice;
        cin >> choice;
        ElementType type = ElementType::None;
        switch (choice) {
        case 1: type = ElementType::Fire; break;
        case 2: type = ElementType::Water; break;
        case 3: type = ElementType::Earth; break;
        case 4: type = ElementType::Air; break;
        default: cout << "Invalid choice. Defaulting to Fire.\n"; type = ElementType::Fire;
        }

        player = make_shared<Player>(name, type);

        enemies.push_back(make_shared<Enemy>("Goblin", ElementType::Earth));
        enemies.push_back(make_shared<Enemy>("Orc", ElementType::Water));
        enemies.push_back(make_shared<Enemy>("Harpy", ElementType::Air));
    }

    void battleLoop() {
        while (player->isAlive() && any_of(enemies.begin(), enemies.end(), [](auto& e) { return e->isAlive(); })) {
            vector<shared_ptr<Character>> turnOrder = { player };
            for (auto& enemy : enemies)
                if (enemy->isAlive()) turnOrder.push_back(enemy);

            sort(turnOrder.begin(), turnOrder.end(),
                [](auto& a, auto& b) { return a->getSpeed() > b->getSpeed(); });

            for (auto& character : turnOrder) {
                if (!character->isAlive()) continue;

                if (character == player) {
                    player->chooseAction(enemies);
                }
                else {
                    dynamic_pointer_cast<Enemy>(character)->takeTurn(player);
                }

                // Grant EXP if enemies die
                for (auto& enemy : enemies) {
                    if (!enemy->isAlive() && enemy->getHealth() > -100) { // Only grant once
                        auto e = dynamic_pointer_cast<Enemy>(enemy);
                        player->gainExperience(e->expReward);
                        enemy->takeDamage(1000); // Prevents double XP
                    }
                }

                if (!player->isAlive() || all_of(enemies.begin(), enemies.end(),
                    [](auto& e) { return !e->isAlive(); })) {
                    break;
                }
            }
        }

        if (player->isAlive()) {
            cout << "\nYou defeated all enemies!\n";
            player->healFull();
            cout << "You are fully healed and restored for the final battle...\n\n";

            // Final boss setup
            enemies.clear();
            enemies.push_back(make_shared<FinalBoss>());
            enemies.push_back(make_shared<Crony>("Golem", ElementType::Earth));
            enemies.push_back(make_shared<Crony>("Wraith", ElementType::Air));

            battleLoop(); // Recursively start final phase
        }

        else {
            cout << "\nYou were defeated!\n";
        }

    }

};

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    Game game;
    game.setup();
    game.battleLoop();
    return 0;
}
