#include <QtWidgets/QApplication>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QStringListModel>
#include <QLabel>
#include <QMutex>
#include <QObject>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

#include <vector>
#include <string>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <windows.h>
#include <chrono>
#include <thread>
#include <future>

#define ALPHABET_SIZE (26) 

// convert letter to corresponding index of alphabet
#define CHAR_TO_INDEX(c) ((int)toupper(c) - (int)'A')

class TrieNode {
public:
    TrieNode();

    void insert(const std::string& key);
    bool search(const std::string& key);
    void suggestionsRec(const QString& currPrefix, QStringList& wordList);
    QStringList printAutoSuggestions(const QString& prefix);

private:
    bool isLastNode();
    

    TrieNode* children[ALPHABET_SIZE];
    bool isWordEnd;
};

TrieNode::TrieNode() {
    isWordEnd = false;
    for (int i = 0; i < ALPHABET_SIZE; i++)
        children[i] = nullptr;
}

void TrieNode::insert(const std::string& key) {
    TrieNode* pCrawl = this;

    for (size_t level = 0; level < key.length(); level++) {
        int index = CHAR_TO_INDEX(key[level]);
        if (!pCrawl->children[index])
            pCrawl->children[index] = new TrieNode();

        pCrawl = pCrawl->children[index];
    }

    pCrawl->isWordEnd = true;
}

bool TrieNode::search(const std::string& key) {
    size_t length = key.length();
    TrieNode* pCrawl = this;

    for (size_t level = 0; level < length; level++) {
        int index = CHAR_TO_INDEX(key[level]);

        if (!pCrawl->children[index])
            return false;

        pCrawl = pCrawl->children[index];
    }

    return (pCrawl != nullptr && pCrawl->isWordEnd);
}

bool TrieNode::isLastNode() {
    for (int i = 0; i < ALPHABET_SIZE; i++)
        if (children[i])
            return false;
    return true;
}

void TrieNode::suggestionsRec(const QString& currPrefix, QStringList& wordList) {
    if (isWordEnd) {
        wordList.append(currPrefix);
    }

    if (isLastNode())
        return;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (children[i]) {
            QString nextPrefix = currPrefix + QChar('A' + i);
            children[i]->suggestionsRec(nextPrefix, wordList);
        }
    }
}

QStringList TrieNode::printAutoSuggestions(const QString& prefix) {
    TrieNode* pCrawl = this;
    QStringList wordList;

    int level;
    int n = prefix.length();
    for (level = 0; level < n; level++) {
        int index = CHAR_TO_INDEX(prefix[level].toLatin1());

        if (!pCrawl->children[index])
            return wordList;

        pCrawl = pCrawl->children[index];
    }

    bool isWord = (pCrawl->isWordEnd == true);
    bool isLast = pCrawl->isLastNode();

    if (isWord && isLast) {
        wordList.append(prefix);
        return wordList;
    }

    if (!isLast) {
        pCrawl->suggestionsRec(prefix, wordList);
    }

    return wordList;
}

class TrieBuilder {
    public:
        TrieBuilder(TrieNode* root, const std::vector<std::string>& words);
        void buildTrie(size_t start, size_t end);

    private:
        TrieNode* root;
        const std::vector<std::string>& words;
};

TrieBuilder::TrieBuilder(TrieNode* rootNode, const std::vector<std::string>& wordList): root(rootNode), words(wordList) {}

void TrieBuilder::buildTrie(size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
        root->insert(words[i]);
    }
}

class WordListUpdater : public QObject {
    Q_OBJECT

    public:
        WordListUpdater(TrieNode* root, QStringListModel* model, QLineEdit* searchBar);
        void updateWordList(const QString& text);

    public slots:
        void onSearchTextChanged(const QString& text);

    signals:
        void searchTimeUpdated(const QString& time);
        void updateListTimeUpdated(const QString& time);

    private:
        TrieNode* root;
        QStringListModel* wordListModel;
        QLineEdit* searchBar;
};

WordListUpdater::WordListUpdater(TrieNode* rootNode, QStringListModel* model, QLineEdit* searchInput): root(rootNode), wordListModel(model), searchBar(searchInput) {
    connect(searchBar, &QLineEdit::textChanged, this, &WordListUpdater::onSearchTextChanged);
}

void WordListUpdater::onSearchTextChanged(const QString& text) {
    QStringList filteredList;

    auto startSearch = std::chrono::high_resolution_clock::now();
    if (text.isEmpty()) {
        root->suggestionsRec("", filteredList);
    }
    else {
        filteredList = root->printAutoSuggestions(text);
    }
    auto stopSearch = std::chrono::high_resolution_clock::now();
    auto searchDuration = std::chrono::duration<double, std::milli>(stopSearch - startSearch);
    emit searchTimeUpdated("Search Time: " + QString::number(searchDuration.count()) + " milliseconds");

    auto startUpdateList = std::chrono::high_resolution_clock::now();
    wordListModel->setStringList(filteredList);
    auto stopUpdateList = std::chrono::high_resolution_clock::now();
    auto updateListDuration = std::chrono::duration<double, std::milli>(stopUpdateList - startUpdateList);
    emit updateListTimeUpdated("Update List Time: " + QString::number(updateListDuration.count()) + " milliseconds");
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Create the main window
    QWidget window;
    window.setWindowTitle("Word List");

    QVBoxLayout* mainLayout = new QVBoxLayout(&window);

    QLabel* searchTimeLabel = new QLabel(&window);
    mainLayout->addWidget(searchTimeLabel);

    QLineEdit* searchBar = new QLineEdit(&window);
    mainLayout->addWidget(searchBar);

    QListView* wordListView = new QListView(&window);
    mainLayout->addWidget(wordListView);

    QLabel* updateListTimeLabel = new QLabel(&window);
    mainLayout->addWidget(updateListTimeLabel);

    std::vector<std::string> words;
    for (char a = 'A'; a <= 'Z'; ++a) {
        for (char b = 'A'; b <= 'Z'; ++b) {
            for (char c = 'A'; c <= 'Z'; ++c) {
                for (char d = 'A'; d <= 'Z'; ++d) {
                    words.push_back(std::string({ a, b, c, d }));
                }
            }
        }
    }
    const size_t numThreadsTmp = 4;
    const size_t numThreads = std::thread::hardware_concurrency();
    const size_t wordsPerThread = words.size() / numThreadsTmp;
    

    TrieNode* root = new TrieNode();
    TrieBuilder trieBuilder(root, words);

    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < numThreadsTmp; ++i) {
        size_t start = i * wordsPerThread;
        size_t end = (i == numThreadsTmp - 1) ? words.size() : start + wordsPerThread;
        futures.push_back(std::async(std::launch::async, &TrieBuilder::buildTrie, &trieBuilder, start, end));
    }

    for (auto& fut : futures) {
        fut.get();
    }

    QStringListModel* model = new QStringListModel();
    QStringList qStringList;
    for (const auto& word : words) {
        qStringList << QString::fromStdString(word);
    }
    root->suggestionsRec("", qStringList);
    model->setStringList(qStringList);
    wordListView->setModel(model);

    WordListUpdater listUpdater(root, model, searchBar);

    QObject::connect(&listUpdater, &WordListUpdater::searchTimeUpdated, [searchTimeLabel](const QString& time) {
        searchTimeLabel->setText(time);
        });

    QObject::connect(&listUpdater, &WordListUpdater::updateListTimeUpdated, [updateListTimeLabel](const QString& time) {
        updateListTimeLabel->setText(time);
        });

    window.show();

    return app.exec();
}

#include "main.moc"
