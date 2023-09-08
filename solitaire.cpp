#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

class SolitaireCipher {
public:
    std::vector<int> deck_;
    SolitaireCipher() : deck_(54), keystream_(0) {
        initializeDeck();
    }

    void initializeDeck() {
        for (int i = 0; i < 54; ++i) {
            deck_[i] = i + 1;
        }
    }

    void performKeySetup(bool saveShuffle = false) {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        for (int i = 53; i > 0; --i) {
            int j = std::rand() % (i + 1);
            std::swap(deck_[i], deck_[j]);
        }
		std::random_shuffle(deck_.begin(), deck_.end());
		if (saveShuffle) {
            std::ofstream shuffleFile("shuffle.txt");
            if (shuffleFile.is_open()) {
                for (int i = 0; i < 54; ++i) {
                    shuffleFile << deck_[i] << " ";
                }
                shuffleFile.close();
                std::cout << "Deck shuffle saved to 'shuffle.txt'" << std::endl;
            } else {
                std::cerr << "Unable to open 'shuffle.txt' for saving shuffle." << std::endl;
            }
        }
    }

    void loadShuffle(const std::string& shuffleFilename) {
        std::ifstream shuffleFile(shuffleFilename.c_str());
        if (shuffleFile.is_open()) {
            for (int i = 0; i < 54; ++i) {
                shuffleFile >> deck_[i];
            }
            shuffleFile.close();
            std::cout << "Deck shuffle loaded from '" << shuffleFilename << "'" << std::endl;
        } else {
            std::cerr << "Unable to open '" << shuffleFilename << "' for loading shuffle." << std::endl;
        }
    }

    int generateKeystreamValue() {
        moveJokerA();
        moveJokerB();
        tripleCut();
        countCut();
        return getOutputValue();
    }

    char encryptChar(char input) {
        if (isalpha(input)) {
            int value = toupper(input) - 'A' + 1;
            int encryptedValue = (value + generateKeystreamValue()) % 26;
            if (encryptedValue <= 0) {
                encryptedValue += 26;
            }
            return static_cast<char>(encryptedValue + 'A' - 1);
        }
        return input; // Non-alphabetic characters remain unchanged
    }

    char decryptChar(char input) {
        if (isalpha(input)) {
            int value = toupper(input) - 'A' + 1;
            int decryptedValue = (value - generateKeystreamValue() + 26) % 26;
            if (decryptedValue <= 0) {
                decryptedValue += 26;
            }
            return static_cast<char>(decryptedValue + 'A' - 1);
        }
        return input; 
    }	

    std::string encrypt(const std::string& plaintext) {
        std::string ciphertext;
        for (char c : plaintext) {
            ciphertext += encryptChar(c);
        }
        return ciphertext;
    }

    std::string decrypt(const std::string& ciphertext) {
        std::string plaintext;
        for (char c : ciphertext) {
            plaintext += decryptChar(c);
        }
        return plaintext;
    }

private:
    int keystream_;

    void moveJokerA() {
        moveJoker(53); 
    }

    void moveJokerB() {
        moveJoker(54);
        moveJoker(54);
    }

    void moveJoker(int jokerValue) {
        int jokerPosition = findCardPosition(jokerValue);
        int newPosition = (jokerPosition + 1) % 54; 
    
        // Special handling for the case when jokerPosition is 54
        if (jokerPosition == 53) {
            deck_[jokerPosition] = deck_[newPosition];
            deck_[newPosition] = jokerValue;
        } else {
            int temp = deck_[jokerPosition];
            deck_[jokerPosition] = deck_[(newPosition + 1) % 54];
            deck_[(newPosition + 1) % 54] = temp;
        }
    }

    int findCardPosition(int cardValue) {
        for (int i = 0; i < deck_.size(); ++i) {
            if (deck_[i] == cardValue) {
                return i;
            }
        }
        return -1; 
    }

    void tripleCut() {
        int firstJoker = findCardPosition(53);
        int secondJoker = findCardPosition(54);
        int lower, upper;

        if (firstJoker > secondJoker) {
            lower = secondJoker;
            upper = firstJoker;
        } else {
            lower = firstJoker;
            upper = secondJoker;
        }

        std::vector<int> topCut(deck_.begin(), deck_.begin() + lower);
        std::vector<int> middleCut(deck_.begin() + lower, deck_.begin() + upper + 1);
        std::vector<int> bottomCut(deck_.begin() + upper + 1, deck_.end());

        deck_.clear();
        deck_.insert(deck_.end(), middleCut.begin(), middleCut.end());
        deck_.insert(deck_.end(), topCut.begin(), topCut.end());
        deck_.insert(deck_.end(), bottomCut.begin(), bottomCut.end());
    }

    void countCut() {
        int lastCard = deck_.back();
        if (lastCard == 53 || lastCard == 54) {
            return; 
        }

        int count = lastCard;
        std::vector<int> topCut(deck_.begin(), deck_.begin() + count);
        std::vector<int> middleCut(deck_.begin() + count, deck_.end() - 1);
        int bottomCard = deck_.back();

        deck_.clear();
        deck_.push_back(bottomCard);
        deck_.insert(deck_.end(), middleCut.begin(), middleCut.end());
        deck_.insert(deck_.end(), topCut.begin(), topCut.end());
    }

    int getOutputValue() {
        int outputValue = deck_[deck_[0]];
        if (outputValue == 53 || outputValue == 54) {
            return generateKeystreamValue(); // Recurse if joker is on top
        }
        return outputValue;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <shuffle_filename>" << std::endl;
        return 1;
    }

    std::string shuffleFilename(argv[1]);
    SolitaireCipher solitaire;
    char choice;

    std::cout << "Do you want to (e)ncrypt or (d)ecrypt? (e/d): ";
    std::cin >> choice;

    if (choice == 'e' || choice == 'E') {
        std::ifstream shuffleFile(shuffleFilename.c_str());
        if (!shuffleFile.is_open()) {
            // Shuffle file doesn't exist; create and save the shuffle during encryption
            solitaire.performKeySetup(true); // Save the shuffle
            std::ofstream newShuffleFile(shuffleFilename.c_str());
            if (newShuffleFile.is_open()) {
                for (int i = 0; i < 54; ++i) {
                    newShuffleFile << solitaire.deck_[i] << ' ';
                }
                newShuffleFile.close();
                std::cout << "Shuffle file '" << shuffleFilename << "' created and saved." << std::endl;
            } else {
                std::cerr << "Unable to create '" << shuffleFilename << "' for saving shuffle." << std::endl;
                return 1;
            }
        } else {
            shuffleFile.close();
            std::cout << "Using existing shuffle file '" << shuffleFilename << "' for encryption." << std::endl;
            solitaire.loadShuffle(shuffleFilename);
        }

        std::string plaintext;
        std::cout << "Enter the plaintext: ";
        std::cin.ignore(); // Ignore newline character from previous input
        std::getline(std::cin, plaintext);

        std::string ciphertext = solitaire.encrypt(plaintext);
        std::cout << "Encrypted text: " << ciphertext << std::endl;
    } else if (choice == 'd' || choice == 'D') {
        solitaire.loadShuffle(shuffleFilename);

        std::string ciphertext;
        std::cout << "Enter the ciphertext to decrypt: ";
        std::cin.ignore(); 
        std::getline(std::cin, ciphertext);

        std::string plaintext = solitaire.decrypt(ciphertext);
        std::cout << "Decrypted text: " << plaintext << std::endl;
    } else {
        std::cerr << "Invalid choice. Please enter 'e' or 'd'." << std::endl;
        return 1;
    }

    return 0;
}
