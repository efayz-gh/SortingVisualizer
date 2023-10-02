#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <random>
#include <iostream>

#include "imgui.h"
#include "imgui-SFML.h"

const unsigned int screenWidth = sf::VideoMode::getDesktopMode().width;
const unsigned int screenHeight = sf::VideoMode::getDesktopMode().height;

sf::RenderWindow window;
sf::Clock deltaClock;

sf::SoundBuffer soundBuffer;
sf::Sound highlightSoundA;
sf::Sound highlightSoundB;

const float maxPitch = 1.5;
const float minPitch = 0.5;

double sleepRatio = 1.0;

void drawArray(sf::RenderTexture &target, std::vector<int> &array, int updateIndexA = -1, int updateIndexB = -1) {

    unsigned int rectWidth = std::max((int) (target.getSize().x / array.size()), 1);
    unsigned int maxRectHeight = target.getSize().y;
    int maxElement = *std::max_element(array.begin(), array.end());

    target.clear();

    // draw rectangles
    for (int i = 0; i < array.size(); i++) {

        sf::RectangleShape rect(sf::Vector2f(rectWidth, (array[i] * maxRectHeight) / maxElement));

        // position rectangles at the bottom
        rect.setPosition(i * rectWidth, maxRectHeight - rect.getSize().y);

        // set color red if updated
        rect.setFillColor(i == updateIndexA || i == updateIndexB ? sf::Color::Red : sf::Color::White);

        target.draw(rect);
    }

    target.display();

    // play sound for highlighted rectangle a
    if (updateIndexA != -1) {
        float pitch = (float) array[updateIndexA] / maxElement;
        pitch = pitch * (maxPitch - minPitch) + minPitch;
        highlightSoundA.setPitch(pitch);

        if (highlightSoundA.getStatus() != sf::Sound::Playing) {
            highlightSoundA.play();
        }
    } else {
        highlightSoundA.pause();
    }

    // play sound for highlighted rectangle b
    if (updateIndexB != -1) {
        float pitch = (float) array[updateIndexB] / maxElement;
        pitch = pitch * (maxPitch - minPitch) + minPitch;
        highlightSoundB.setPitch(pitch);

        if (highlightSoundB.getStatus() != sf::Sound::Playing) {
            highlightSoundB.play();
        }
    } else {
        highlightSoundB.pause();
    }
}

void stopButtonWindow() {
    ImGui::Begin("stop", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(window.getSize().x / 8, window.getSize().y / 8));

    if (ImGui::Button("stop")) {
        ImGui::End();

        highlightSoundA.pause();
        highlightSoundB.pause();

        // using exception to break out of multiple function calls
        throw std::exception();
    }
    ImGui::End();
}

void visualize(std::vector<int> &array, sf::Time delay, int updateIndexA = -1, int updateIndexB = -1) {

    delay = sf::microseconds(delay.asMicroseconds() / sleepRatio);

    sf::RenderTexture windowTexture;
    windowTexture.create(window.getSize().x, window.getSize().y);

    drawArray(windowTexture, array, updateIndexA, updateIndexB);

    sf::Sprite visualization(windowTexture.getTexture());

    sf::Clock clock{};
    do {
        sf::Event event{};
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    exit(0);
                case sf::Event::Resized:
                    window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // stop button
        stopButtonWindow();

        window.clear();
        window.draw(visualization);
        ImGui::SFML::Render(window);
        window.display();

    } while (clock.getElapsedTime() < delay);
}

void visualizeWait(std::vector<int> &array, sf::Time delay) {
    double tmpRatio = sleepRatio;
    sleepRatio = 1.0;
    visualize(array, delay);
    sleepRatio = tmpRatio;
}

void writeVisualize(std::vector<int> &src, std::vector<int> &dst, sf::Time delay) {
    for (int i = 0; i < src.size(); i++) {
        dst[i] = src[i];
        visualize(dst, delay, i);
    }
}

void shuffle(std::vector<int> &array, sf::Time delay = sf::milliseconds(1)) {

    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < array.size(); i++) {
        std::uniform_int_distribution<> dis(0, array.size() - 1);
        int rand = dis(gen);

        std::swap(array[i], array[rand]);
        visualize(array, delay, i, rand);
    }
}

void bubbleSort(std::vector<int> &array, sf::Time delay = sf::milliseconds(10)) {

    for (int i = 0; i < array.size(); i++) {

        int lastSwapIndex = 0; // for visualization

        for (int j = 0; j < array.size() - i - 1; j++) {
            if (array[j] > array[j + 1]) {
                std::swap(array[j], array[j + 1]);
                lastSwapIndex = j;
            }
        }

        visualize(array, delay, array.size() - i - 1, lastSwapIndex);
    }
}

void insertionSort(std::vector<int> &array, sf::Time delay = sf::milliseconds(10)) {

    for (int i = 1; i < array.size(); i++) {

        int j;
        for (j = i; j > 0 && array[j] < array[j - 1]; j--) {
            std::swap(array[j], array[j - 1]);
        }

        visualize(array, delay, j - 1, i);
    }
}

void selectionSort(std::vector<int> &array, sf::Time delay = sf::microseconds(25)) {

    for (int i = 0; i < array.size(); i++) {

        int minIndex = i;
        for (int j = i + 1; j < array.size(); j++) {
            if (array[j] < array[minIndex]) {
                minIndex = j;

                visualize(array, delay, i, minIndex);
            }
            visualize(array, delay, i, j);
        }

        std::swap(array[i], array[minIndex]);
        visualize(array, delay, i, minIndex);
    }
}

void heapSort(std::vector<int> &array, sf::Time delay = sf::microseconds(500)) {

    for (int i = 1; i < array.size(); i++) {

        int childIndex = i;
        int parentIndex = (childIndex - 1) / 2;

        while (array[childIndex] < array[parentIndex]) {
            std::swap(array[childIndex], array[parentIndex]);
            visualize(array, delay, childIndex, parentIndex);

            childIndex = parentIndex;
            parentIndex = (childIndex - 1) / 2;
        }
    }

    for (int i = array.size() - 1; i > 0; i--) {

        std::swap(array[0], array[i]);
        visualize(array, delay, 0, i);

        int parentIndex = 0;
        int leftChildIndex = 2 * parentIndex + 1;
        int rightChildIndex = 2 * parentIndex + 2;

        while (leftChildIndex < i) {

            int minIndex = parentIndex;

            if (array[leftChildIndex] < array[minIndex]) {
                minIndex = leftChildIndex;
            }

            if (rightChildIndex < i && array[rightChildIndex] < array[minIndex]) {
                minIndex = rightChildIndex;
            }

            if (minIndex == parentIndex) {
                break;
            }

            std::swap(array[parentIndex], array[minIndex]);
            visualize(array, delay, parentIndex, minIndex);

            parentIndex = minIndex;
            leftChildIndex = 2 * parentIndex + 1;
            rightChildIndex = 2 * parentIndex + 2;
        }
    }

    for (int i = 0; i < array.size() / 2; i++) {
        std::swap(array[i], array[array.size() - i - 1]);
        visualize(array, delay, i, array.size() - i - 1);
    }
}

void mergeSort(std::vector<int> &array, sf::Time delay = sf::microseconds(500)) {

    std::vector<int> temp(array.size());

    for (int width = 1; width < array.size(); width *= 2) {

        for (int i = 0; i < array.size(); i += 2 * width) {

            int left = i;
            int middle = std::min(i + width, (int) array.size());
            int right = std::min(i + 2 * width, (int) array.size());

            int leftIndex = left;
            int rightIndex = middle;

            for (int j = left; j < right; j++) {

                if (leftIndex < middle && (rightIndex >= right || array[leftIndex] < array[rightIndex])) {
                    temp[j] = array[leftIndex];
                    leftIndex++;
                } else {
                    temp[j] = array[rightIndex];
                    rightIndex++;
                }

                visualize(array, delay, j);
            }
        }

        writeVisualize(temp, array, delay);
        visualize(array, sf::Time::Zero);
    }
}

void radixSort(std::vector<int> &array, sf::Time delay = sf::microseconds(500)) {

    std::vector<int> temp(array.size());

    int maxElement = *std::max_element(array.begin(), array.end());

    for (int exp = 1; maxElement / exp > 0; exp *= 10) {

        std::vector<int> count(10);

        for (int i: array) {
            count[(i / exp) % 10]++;
        }

        for (int i = 1; i < count.size(); i++) {
            count[i] += count[i - 1];
        }

        for (int i = array.size() - 1; i >= 0; i--) {
            temp[count[(array[i] / exp) % 10] - 1] = array[i];
            count[(array[i] / exp) % 10]--;
        }

        writeVisualize(temp, array, delay);
        visualize(array, sf::Time::Zero);
    }
}

int main() {

    // create window
    window.create(sf::VideoMode(screenWidth / 2, screenHeight / 2), "sorting");

    // initialize ImGui
    if (!ImGui::SFML::Init(window))
        return 1;

    // load sound
    if (!soundBuffer.loadFromFile("sort.wav"))
        return 1;

    highlightSoundA.setBuffer(soundBuffer);
    highlightSoundB.setBuffer(soundBuffer);
    highlightSoundA.setVolume(10);
    highlightSoundB.setVolume(10);
    highlightSoundA.setLoop(true);
    highlightSoundB.setLoop(true);

    // initialize array
    int arraySize = 256;
    int lastArraySize = arraySize;

    std::vector<int> array = std::vector<int>(arraySize);
    for (int i = 0; i < array.size(); i++) {
        array[i] = i + 1;
    }

    // main loop
    while (window.isOpen()) {

        // limit framerate to 30 to reduce CPU/GPU usage in controls window
        window.setFramerateLimit(30);

        // handle events
        sf::Event event{};
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                case sf::Event::Resized:
                    window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
            }
        }
        ImGui::SFML::Update(window, deltaClock.restart());

        // start of controls window
        ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoCollapse);

        // algorithm combo box items
        const char *algorithms[] = {
                "Bubble Sort",
                "Insertion Sort",
                "Selection Sort",
                "Heap Sort",
                "Merge Sort",
                "Radix Sort",
        };

        // algorithm combo box
        static int algorithm = 0;
        ImGui::Combo("Algorithm", &algorithm, algorithms, IM_ARRAYSIZE(algorithms));

        // array size input
        ImGui::InputInt("Array Size", &arraySize, 1, 4);
        arraySize = std::max(2, std::min(arraySize, 1024));

        // resize array if array size changed
        if (arraySize != lastArraySize) {
            array = std::vector<int>(arraySize);
            for (int i = 0; i < array.size(); i++) {
                array[i] = i + 1;
            }
            lastArraySize = arraySize;
        }

        // visualize button
        if (ImGui::Button("Visualize", ImVec2(100, 20))) {
            ImGui::End(); // end controls window early because it is unneeded during visualization

            // finish rendering controls window
            window.clear();
            ImGui::SFML::Render(window);
            window.display();

            // remove framerate limit for sorting as it limits the speed of the sorting (disadvantage of single thread)
            window.setFramerateLimit(0);

            sleepRatio = arraySize / 1024.0;

            try {

                visualizeWait(array, sf::seconds(1));

                shuffle(array);

                visualizeWait(array, sf::seconds(1));

                switch (algorithm) {
                    case 0:
                        bubbleSort(array);
                        break;
                    case 1:
                        insertionSort(array);
                        break;
                    case 2:
                        selectionSort(array);
                        break;
                    case 3:
                        heapSort(array);
                        break;
                    case 4:
                        mergeSort(array);
                        break;
                    case 5:
                        radixSort(array);
                        break;
                    default:
                        break;
                }

                visualizeWait(array, sf::seconds(1));

            } catch (std::exception &e) {
                // do nothing because exception is thrown by stop button
            }
        } else {
            ImGui::End();
        }

        //ImGui::ShowDemoWindow();

        // draw array
        sf::RenderTexture windowTexture;
        windowTexture.create(window.getSize().x, window.getSize().y);
        drawArray(windowTexture, array);

        window.draw(sf::Sprite(windowTexture.getTexture()));

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
