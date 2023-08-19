#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <random>

#include "imgui.h"
#include "imgui-SFML.h"

unsigned int screenWidth = sf::VideoMode::getDesktopMode().width;
unsigned int screenHeight = sf::VideoMode::getDesktopMode().height;

sf::RenderWindow window(sf::VideoMode(screenWidth / 2, screenHeight / 2), "sorting");
sf::Clock deltaClock;

sf::SoundBuffer soundBuffer;
sf::Sound highlightSoundA;
sf::Sound highlightSoundB;

const float maxPitch = 1.5;
const float minPitch = 0.5;

void visualize(std::vector<int> &array, int updateIndexA = -1, int updateIndexB = -1) {

    unsigned int rectWidth = std::max((int) (window.getSize().x / array.size()), 1);
    unsigned int maxRectHeight = window.getSize().y;
    int maxElement = *std::max_element(array.begin(), array.end());

    window.clear(sf::Color::Black);

    // draw rectangles
    for (int i = 0; i < array.size(); i++) {

        sf::RectangleShape rect(sf::Vector2f(rectWidth, (array[i] * maxRectHeight) / maxElement));

        // position rectangles at the bottom
        rect.setPosition(i * rectWidth, maxRectHeight - rect.getSize().y);

        // set color red if updated
        rect.setFillColor(i == updateIndexA || i == updateIndexB ? sf::Color::Red : sf::Color::White);

        window.draw(rect);
    }

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

    ImGui::SFML::Render(window);
    window.display();

    ImGui::SFML::Update(window, deltaClock.restart());
}

// custom sleep function that allows for window events to be processed
void sleepPoll(sf::Time delay) {
    sf::Clock clock;
    while (clock.getElapsedTime() < delay) {
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
    }
}

void writeVisualize(std::vector<int> &src, std::vector<int> &dst, sf::Time delay) {
    for (int i = 0; i < src.size(); i++) {
        dst[i] = src[i];
        visualize(dst, i);
        sleepPoll(delay);
    }
}

void shuffle(std::vector<int> &array, sf::Time delay = sf::milliseconds(1)) {

    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < array.size(); i++) {
        std::uniform_int_distribution<> dis(0, array.size() - 1);
        int rand = dis(gen);

        std::swap(array[i], array[rand]);
        visualize(array, i, rand);
        sleepPoll(delay);
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

        visualize(array, array.size() - i - 1, lastSwapIndex);
        sleepPoll(delay);
    }
}

void insertionSort(std::vector<int> &array, sf::Time delay = sf::milliseconds(10)) {

    for (int i = 0; i < array.size(); i++) {

        int j;
        for (j = i; j > 0 && array[j] < array[j - 1]; j--) {
            std::swap(array[j], array[j - 1]);
        }

        visualize(array, j - 1, i);
        sleepPoll(delay);
    }
}

void selectionSort(std::vector<int> &array, sf::Time delay = sf::microseconds(25)) {

    for (int i = 0; i < array.size(); i++) {

        int minIndex = i;
        for (int j = i + 1; j < array.size(); j++) {
            if (array[j] < array[minIndex]) {
                minIndex = j;

                visualize(array, i, minIndex);
                sleepPoll(delay);
            }
        }

        std::swap(array[i], array[minIndex]);
        visualize(array, i, minIndex);
        sleepPoll(delay);

    }
}

void inplaceHeapSort(std::vector<int> &array, sf::Time delay = sf::microseconds(500)) {

    for (int i = 1; i < array.size(); i++) {

        int childIndex = i;
        int parentIndex = (childIndex - 1) / 2;

        while (array[childIndex] < array[parentIndex]) {
            std::swap(array[childIndex], array[parentIndex]);
            visualize(array, childIndex, parentIndex);
            sleepPoll(delay);

            childIndex = parentIndex;
            parentIndex = (childIndex - 1) / 2;
        }
    }

    for (int i = array.size() - 1; i > 0; i--) {

        std::swap(array[0], array[i]);
        visualize(array, 0, i);
        sleepPoll(delay);

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
            visualize(array, parentIndex, minIndex);
            sleepPoll(delay);

            parentIndex = minIndex;
            leftChildIndex = 2 * parentIndex + 1;
            rightChildIndex = 2 * parentIndex + 2;
        }
    }

    for (int i = 0; i < array.size() / 2; i++) {
        std::swap(array[i], array[array.size() - i - 1]);
        visualize(array, i, array.size() - i - 1);
        sleepPoll(delay);
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

                visualize(array, j);
                sleepPoll(delay);
            }
        }

        writeVisualize(temp, array, delay);
        visualize(array);
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
        visualize(array);
        sleepPoll(delay);
    }
}

int main() {

    // initialize ImGui
    if (!ImGui::SFML::Init(window))
        return 1;

    // load sound
    if (!soundBuffer.loadFromFile("1000hz.wav"))
        return 1;

    highlightSoundA.setBuffer(soundBuffer);
    highlightSoundB.setBuffer(soundBuffer);
    highlightSoundA.setVolume(10);
    highlightSoundB.setVolume(10);
    highlightSoundA.setLoop(true);
    highlightSoundB.setLoop(true);

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
        static int arraySize = 256;
        ImGui::InputInt("Array Size", &arraySize, 1, 4);
        arraySize = std::max(2, std::min(arraySize, 1024));

        // visualize button
        if (ImGui::Button("Visualize", ImVec2(100, 20))) {
            ImGui::End(); // end controls window early because it is unneeded during visualization

            window.clear(sf::Color::Black);
            ImGui::SFML::Render(window);
            window.display();
            ImGui::SFML::Update(window, deltaClock.restart());

            std::vector<int> array = std::vector<int>(arraySize);
            for (int i = 0; i < array.size(); i++) {
                array[i] = i + 1;
            }

            // remove framerate limit for sorting as it limits the speed of the sorting (disadvantage of single thread)
            window.setFramerateLimit(0);

            try {
                visualize(array);
                sleepPoll(sf::seconds(1));

                shuffle(array);

                visualize(array);
                sleepPoll(sf::seconds(1));

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
                        inplaceHeapSort(array);
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

                visualize(array);
                sleepPoll(sf::seconds(1));

            } catch (std::exception &e) {
                // do nothing because exception is thrown by stop button
            }
        } else {
            ImGui::End();
        }

        //ImGui::ShowDemoWindow();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
