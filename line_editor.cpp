#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <stack>

using namespace std;
using namespace std::filesystem;

struct Node
{
    string word;
    Node *next;
    Node(string word) : word(word), next(nullptr) {}
};

struct Operation
{
    string type;
    int lineNumber;
    int wordNumber;
    string newData;
    string oldData;
    int startIndex;
    int endIndex;
};

class LinkedList
{
public:
    Node *head;
    LinkedList() : head(nullptr) {}

    int wordCount()
    {
        int count = 0;
        Node *current = head;

        while (current != nullptr)
        {
            count++;
            current = current->next;
        }

        return count;
    }

    int find(string &word)
    {
        Node *current = head;
        int i = 0;
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        while (current != nullptr)
        {
            string listWord = current->word;
            transform(listWord.begin(), listWord.end(), listWord.begin(), ::tolower);
            if (listWord == word)
            {
                return i;
            }
            current = current->next;
            i++;
        }
        return -1;
    }

    void insertWordAt(int position, const string &newWord)
    {
        Node *newNode = new Node(newWord);
        // If insert at head
        if (position == 0)
        {
            newNode->next = head;
            head = newNode;
            return;
        }

        Node *current = head;
        int index = 0;
        while (current != nullptr && index < position - 1)
        {
            current = current->next;
            index++;
        }
        if (current == nullptr)
        {
            cout << "Invalid position!" << endl;
            return;
        }

        newNode->next = current->next;
        current->next = newNode;
    }

    void removeWordAt(int position)
    {
        if (head == nullptr)
            return;
        if (position == 0)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
            return;
        }

        Node *current = head;
        int index = 0;
        while (current != nullptr && index < position - 1)
        {
            current = current->next;
            index++;
        }
        if (current == nullptr || current->next == nullptr)
        {
            cout << "Invalid position!" << endl;
            return;
        }

        Node *temp = current->next;
        current->next = current->next->next;
        delete temp;
    }

    void display()
    {
        Node *current = head;
        while (current != nullptr)
        {
            cout << current->word << " ";
            current = current->next;
        }
    }

    string toString()
    {
        Node *current = head;
        string str;
        while (current != nullptr)
        {
            str += current->word;
            if (current->next != nullptr)
            {
                str += " ";
            }
            current = current->next;
        }
        return str;
    }

    string getWord(int index)
    {
        Node *current = head;
        int i = 0;
        while (current != nullptr && i != index)
        {
            current = current->next;
            i++;
        }
        if (current == nullptr)
        {
            return "";
        }
        return current->word;
    }

    vector<string> toVector()
    {
        vector<string> words;
        Node *current = head;
        while (current != nullptr)
        {
            words.push_back(current->word);
            current = current->next;
        }
        return words;
    }

    void fromVector(const vector<string> &words)
    {
        head = nullptr;
        for (const string &word : words)
        {
            insertWordAt(0, word); // Insert at head for simplicity
        }
    }

    void clear()
    {
        while (head != nullptr)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
        }
    }
};

class LineEditor
{
private:
    vector<LinkedList> buffer;
    string fileName;
    int choice, pos;
    string newLine, newWord, word;
    string newContent;
    int cursorLine = 1;
    int cursorWord = 0;
    string tillWord;
    int flag;
    stack<Operation> undoStack;
    stack<Operation> redoStack;
    int undoRedoFlag = 0;

public:
    LineEditor(string &filePath)
    {
        fileName = filePath;
    }

    void loadFile()
    {
        ifstream file(fileName);
        if (!file.is_open())
        {
            cout << "Error: Unable to open file.\n";
            return;
        }

        string line;
        while (getline(file, line))
        {
            LinkedList lineList;
            stringstream ss(line);
            string word;
            int i = 0;
            while (ss >> word)
            {
                lineList.insertWordAt(i, word);
                i++;
            }
            buffer.push_back(lineList);
        }
        file.close();

        if (!buffer.empty())
        {
            cursorLine = buffer.size() + 1;
            cursorWord = 0;
            cout << "Cursor initialized at Line: " << cursorLine << ", Word: " << cursorWord << endl;
        }
    }

    void displayLines()
    {
        if (buffer.empty())
        {
            cout << "No content available.\n";
            return;
        }

        cout << endl;
        cout << "File Content: " << endl;
        for (int i = 0; i < buffer.size(); i++)
        {
            cout << i + 1 << ". ";
            buffer[i].display();
            cout << endl;
        }
    }

    void displaySpecificLine(int lineNumber)
    {
        if (lineNumber < 1 || lineNumber > buffer.size())
        {
            cout << "Error: Invalid line number.\n";
            return;
        }

        cout << "Line " << lineNumber << ": ";
        buffer[lineNumber - 1].display();
        cout << endl;
    }

    void insertLine(int pos, string &newLine)
    {
        if (pos < 1 || pos > buffer.size() + 1)
        {
            cout << "Error: Invalid position" << endl;
            return;
        }
        LinkedList newLineList;
        stringstream ss(newLine);
        string word;
        int i = 0;
        while (ss >> word)
        {
            newLineList.insertWordAt(i, word);
            i++;
        }
        if (!undoRedoFlag)
        {
            // Save operation
            undoStack.push({"insertLine", pos, 0, newLine, "", 0, 0});
            while (!redoStack.empty())
                redoStack.pop();
        }

        buffer.insert(buffer.begin() + (pos - 1), newLineList);
        cursorLine = pos;
        cursorWord = 0;
        cout << "Line inserted at position " << pos << endl;
    }

    // Revisit
    void insertWord(string &newWord)
    {
        if (cursorLine < 1 || cursorLine > buffer.size())
        {
            cout << "Inavlid cursor position for line." << endl;
            return;
        }
        LinkedList &line = buffer[cursorLine - 1];

        if (cursorWord < 0 || cursorWord > line.wordCount())
        {
            cout << "Error: Invalid cursor position for word" << endl;
            return;
        }

        if (!undoRedoFlag)
        {
            // Record operation in stack
            undoStack.push({"insertWord", cursorLine, cursorWord, newWord, "", 0, 0});
            while (!redoStack.empty())
                redoStack.pop();
        }

        line.insertWordAt(cursorWord, newWord);

        cout << "Word inserted successfully!!" << endl;
    }

    void saveLine(string &fileName)
    {
        ofstream file(fileName);
        if (!file.is_open())
        {
            cout << "Error: Unable to save file!!" << endl;
            return;
        }
        for (LinkedList &line : buffer)
        {
            Node *current = line.head;
            while (current != nullptr)
            {
                file << current->word;
                if (current->next != nullptr)
                {
                    file << " ";
                }
                current = current->next;
            }
            file << endl;
        }
        file.close();

        while (!undoStack.empty())
        {
            undoStack.pop();
        }

        while (!redoStack.empty())
        {
            redoStack.pop();
        }

        cout << "File saved successfully!!" << endl;
    }

    void deleteLine(int pos)
    {
        if (pos < 1 || pos > buffer.size())
        {
            cout << "Invalid position!!" << endl;
            return;
        }

        LinkedList deleteLine = buffer[pos - 1];
        string lineContent = deleteLine.toString();

        if (!undoRedoFlag)
        {
            // Store deleted line
            undoStack.push({"deleteLine", pos, 0, lineContent, "", 0, 0});
            while (!redoStack.empty())
            {
                redoStack.pop();
            }
        }

        buffer[pos - 1].clear();
        buffer.erase(buffer.begin() + (pos - 1)); // Cannot pass (pos-1) directly in erase function because it takes an iterator and and buffer.begin() is an iterator
        cout << "Lines deleted successfully!!" << endl;
    }

    void deleteWord()
    {
        if (cursorLine < 1 || cursorLine > buffer.size())
        {
            cout << "Invalid line index.\n";
            return;
        }

        LinkedList &line = buffer[cursorLine - 1];

        if (cursorWord < 0 || cursorWord >= line.wordCount())
        {
            cout << "Invalid word index.\n";
            return;
        }

        if (line.wordCount() == 1)
        {
            deleteLine(cursorLine);
            return;
        }

        string deletedWord = line.getWord(cursorWord);
        if (deletedWord.empty())
        {
            cout << "Error: Invalid cursor position." << endl;
            return;
        }

        if (!undoRedoFlag)
        {
            // Store operation in stack
            undoStack.push({"deleteWord", cursorLine, cursorWord, deletedWord, "", 0, 0});
            while (!redoStack.empty())
            {
                redoStack.pop();
            }
        }

        line.removeWordAt(cursorWord);

        cout << "Word deleted successfully.\n";
    }

    void modifyLine(int pos, string &newLine)
    {
        if (pos < 1 || pos > buffer.size())
        {
            cout << "Error: Invalid position.\n";
            return;
        }

        // Old content of the line
        string oldLine = buffer[pos - 1].toString();

        if (!undoRedoFlag)
        {
            // Store operation in stack
            undoStack.push({"modifyLine", pos, 0, newLine, oldLine, 0, 0});
            while (!redoStack.empty())
            {
                redoStack.pop();
            }
        }

        // String to LinkedList
        LinkedList newLineList;
        stringstream ss(newLine);
        string word;
        int i = 0;
        while (ss >> word)
        {
            newLineList.insertWordAt(i, word);
            i++;
        }

        buffer[pos - 1] = newLineList;
        cout << "Line modified successfully.\n";
    }

    // Revisit
    void modifyLineRange(string &newContent, string till)
    {
        if (cursorLine < 1 || cursorLine > buffer.size())
        {
            cout << "Error: Invalid cursor position for the line." << endl;
            return;
        }

        LinkedList &line = buffer[cursorLine - 1];

        if (cursorWord < 0 || cursorWord >= line.wordCount())
        {
            cout << "Error: Inavlid cursor position for word." << endl;
            return;
        }

        int tillIndex = -1;

        // If index given
        if (isdigit(till[0]))
        {
            tillIndex = stoi(till);
            if (tillIndex < cursorWord || tillIndex >= line.wordCount())
            {
                cout << "Error: Invalid index given." << endl;
                return;
            }
        }
        else
        {
            // If word given
            tillIndex = line.find(till);
            if (tillIndex == -1)
            {
                cout << "Error: Word not found." << endl;
                return;
            }
        }

        // Get old content
        string oldContent;
        for (int j = cursorWord; j <= tillIndex; j++)
        {
            oldContent += line.getWord(j);
            if (j != tillIndex)
            {
                oldContent += " ";
            }
        }

        if (!undoRedoFlag)
        {
            // Store operation in stack
            undoStack.push({"modifyLineRange", cursorLine, cursorWord, newContent, oldContent, cursorWord, tillIndex});
            while (!redoStack.empty())
            {
                redoStack.pop();
            }
        }

        for (int j = 0; j <= tillIndex - cursorWord; j++)
        {
            line.removeWordAt(cursorWord);
        }
        stringstream ss(newContent);
        string word;
        while (ss >> word)
        {
            line.insertWordAt(cursorWord, word);
            cursorWord++;
        }

        cout << "Line modified successfully!!" << endl;
    }

    void replaceWord(string &oldWord, string &newWord)
    {
        if (cursorLine < 1 || cursorLine > buffer.size())
        {
            cout << "Error: Invalid cursor position for line." << endl;
            return;
        }

        LinkedList &line = buffer[cursorLine - 1];
        int pos = line.find(oldWord);

        if (pos == -1)
        {
            cout << "Error: Word not found." << endl;
            return;
        }

        Node *current = line.head;
        int index = 0;

        while (current != nullptr && index < pos)
        {
            current = current->next;
            index++;
        }

        if (current != nullptr)
        {
            if (!undoRedoFlag)
            {
                // Store operation in stack
                undoStack.push({"replaceWord", cursorLine, pos, newWord, oldWord, 0, 0});
                while (!redoStack.empty())
                {
                    redoStack.pop();
                }
            }

            current->word = newWord;
            cout << "Word replaced successfully!!" << endl;
        }
        else
        {
            cout << "Error: Couldn't find the word at the specified position." << endl;
        }
    }

    void searchWord(string word)
    {
        for (int i = 0; i < buffer.size(); i++)
        {
            LinkedList &line = buffer[i];
            int wordPos = line.find(word);
            if (wordPos != -1)
            {
                cout << "Word found at Line: " << i + 1 << ", Word Position: " << wordPos << endl;
                return;
            }
        }
        cout << "Word not found!" << endl;
    }

    // Stack operations
    void undo()
    {
        if (undoStack.empty())
        {
            cout << "No operation to undo." << endl;
            return;
        }
        Operation op = undoStack.top();
        undoStack.pop();
        redoStack.push(op);

        if (op.type == "insertWord")
        {
            cursorLine = op.lineNumber;
            cursorWord = op.wordNumber;
            undoRedoFlag = 1;
            deleteWord();
            undoRedoFlag = 0;
        }
        else if (op.type == "insertLine")
        {
            undoRedoFlag = 1;
            deleteLine(op.lineNumber);
            undoRedoFlag = 0;
        }
        else if (op.type == "deleteLine")
        {
            undoRedoFlag = 1;
            insertLine(op.lineNumber, op.newData);
            undoRedoFlag = 0;
        }
        else if (op.type == "deleteWord")
        {
            cursorLine = op.lineNumber;
            cursorWord = op.wordNumber;
            undoRedoFlag = 1;
            insertWord(op.newData);
            undoRedoFlag = 0;
        }
        else if (op.type == "modifyLine")
        {
            undoRedoFlag = 1;
            modifyLine(op.lineNumber, op.oldData);
            undoRedoFlag = 0;
        }
        else if (op.type == "modifyLineRange")
        {
            cursorLine = op.lineNumber;
            cursorWord = op.wordNumber;
            undoRedoFlag = 1;
            modifyLineRange(op.oldData, to_string(op.endIndex - 1));
            undoRedoFlag = 0;
        }
        else if (op.type == "replaceWord")
        {
            cursorLine = op.lineNumber;
            cursorWord = op.wordNumber;
            undoRedoFlag = 1;
            replaceWord(op.newData, op.oldData);
            undoRedoFlag = 0;
        }

        cout << endl;
        cout << "Undo Successfull!!" << endl;
    }

    void redo()
    {
        if (redoStack.empty())
        {
            cout << "No operation to redo." << endl;
            return;
        }

        // Pop operation from redo and push into undo
        Operation op = redoStack.top();
        redoStack.pop();
        undoStack.push(op);

        if (op.type == "insertWord")
        {
            cursorWord = op.wordNumber;
            undoRedoFlag = 1;
            insertWord(op.newData);
            undoRedoFlag = 0;
        }
        else if (op.type == "insertLine")
        {
            undoRedoFlag = 1;
            insertLine(op.lineNumber, op.newData);
            undoRedoFlag = 0;
        }
        else if (op.type == "deleteLine")
        {
            undoRedoFlag = 1;
            deleteLine(op.lineNumber);
            undoRedoFlag = 0;
        }
        else if (op.type == "deleteWord")
        {
            cursorLine = op.lineNumber;
            cursorWord = op.wordNumber;
            undoRedoFlag = 1;
            deleteWord();
            undoRedoFlag = 0;
        }
        else if (op.type == "modifyLine")
        {
            undoRedoFlag = 1;
            modifyLine(op.lineNumber, op.newData);
            undoRedoFlag = 0;
        }
        else if (op.type == "modifyLineRange")
        {
            cursorLine = op.lineNumber;
            cursorWord = op.wordNumber;
            undoRedoFlag = 1;
            modifyLineRange(op.newData, to_string(op.endIndex));
            undoRedoFlag = 0;
        }
        else if (op.type == "replaceWord")
        {
            cursorLine = op.lineNumber;
            cursorWord = op.wordNumber;
            undoRedoFlag = 1;
            replaceWord(op.oldData, op.newData);
            undoRedoFlag = 0;
        }

        cout << "Redo successful!" << endl;
    }

    void run()
    {
        loadFile();

        do
        {
            cout << endl;
            cout << "1. Display file content" << endl;
            cout << "2. Display specific line" << endl;
            cout << "3. Insert line" << endl;
            cout << "4. Insert word" << endl;
            cout << "5. Move cursor" << endl;
            cout << "6. Delete line" << endl;
            cout << "7. Delete word" << endl;
            cout << "8. Modify line" << endl;
            cout << "9. Modify line in range" << endl;
            cout << "10. Replace a word" << endl;
            cout << "11. Search a word" << endl;
            cout << "12. Undo" << endl;
            cout << "13. Redo" << endl;
            cout << "14. Save" << endl;
            cout << "15. Exit" << endl;
            cout << "Enter your choice" << endl;
            cin >> choice;

            switch (choice)
            {
            case 1:
                displayLines();
                break;
            case 2:
                cout << "Enter line number to display: ";
                cin >> pos;
                displaySpecificLine(pos);
                break;
            case 3:
                cout << "Enter new line: " << endl;
                cin.ignore();
                getline(cin, newLine);
                cout << "Do you want to provide the cursor position for line? True(1) or False(0)" << endl;
                cin >> flag;
                if (flag)
                {
                    cout << "Enter the position where you want to enter the line: " << endl;
                    cin >> cursorLine;
                }
                insertLine(cursorLine, newLine);
                cursorLine++;
                flag = 0;
                break;
            case 4:
                if (buffer.empty())
                {
                    cout << "No line available to insert word.\n";
                    break;
                }
                cout << "Enter new word: ";
                cin >> newWord;
                cout << "Do you want to provide the cursor position for line and word? True(1) or False(0)" << endl;
                cin >> flag;
                if (flag)
                {
                    cout << "Enter the cursor position (line): " << endl;
                    cin >> cursorLine;
                    cout << "Enter the cursor position (word [0-indexed]): " << endl;
                    cin >> cursorWord;
                }
                insertWord(newWord);
                cursorWord++;
                flag = 0;
                break;
            case 5:
                cout << "Enter new cursor position for line: ";
                cin >> cursorLine;
                cout << "Enter new cursor position for word (0-indexed): ";
                cin >> cursorWord;
                cout << "Cursor moved to Line: " << cursorLine << ", Word: " << cursorWord << endl;
                break;
            case 6:
                if (buffer.empty())
                {
                    cout << "No content available to delete.\n";
                    break;
                }
                cout << "Enter position to delete: ";
                cin >> pos;
                deleteLine(pos);
                break;
            case 7:
                if (buffer.empty())
                {
                    cout << "No content available to delete.\n";
                    break;
                }
                cout << "Do you want to provide the cursor position for line? True(1) or False(0)" << endl;
                cin >> flag;
                if (flag)
                {
                    cout << "Enter the cursor position (line): " << endl;
                    cin >> cursorLine;
                }
                cout << "Enter word index to delete" << endl;
                cin >> cursorWord;
                deleteWord();
                flag = 0;
                break;
            case 8:
                if (buffer.empty())
                {
                    cout << "No content available to modify.\n";
                    break;
                }
                cout << "Enter line number to modify: ";
                cin >> pos;
                cin.ignore();
                cout << "Enter new line: ";
                getline(cin, newContent);
                modifyLine(pos, newContent);
                break;
            case 9:
                if (buffer.empty())
                {
                    cout << "No content available to modify.\n";
                    break;
                }
                cout << "Do you want to provide the cursor position for line? True(1) or False(0)" << endl;
                cin >> flag;
                if (flag)
                {
                    cout << "Enter the cursor position (line): " << endl;
                    cin >> cursorLine;
                }
                cout << "Enter the starting cursor word position: ";
                cin >> cursorWord;
                cout << "Enter the ending word (or index) to modify to: ";
                cin >> tillWord;
                cout << "Enter new content for the line starting from cursor: \n";
                cin.ignore();
                getline(cin, newContent);
                modifyLineRange(newContent, tillWord);
                flag = 0;
                break;
            case 10:
                if (buffer.empty())
                {
                    cout << "No content available to replace.\n";
                    break;
                }
                cout << "Do you want to provide the cursor position for line? True(1) or False(0)" << endl;
                cin >> flag;
                if (flag)
                {
                    cout << "Enter the cursor position (line): " << endl;
                    cin >> cursorLine;
                }
                cout << "Enter word to replace: " << endl;
                cin >> newWord;
                cout << "Enter new word: " << endl;
                cin >> newContent;
                replaceWord(newWord, newContent);
                break;
            case 11:
                if (buffer.empty())
                {
                    cout << "No content available to search.\n";
                    break;
                }
                cout << "Enter the word you want to search: " << endl;
                cin >> word;
                searchWord(word);
                break;
            case 12:
                undo();
                break;
            case 13:
                redo();
                break;
            case 14:
                saveLine(fileName);
                break;
            case 15:
                break;
            default:
                cout << "Invalid choice. Please try again" << endl;
                break;
            }
        } while (choice >= 1 && choice < 15);
    }
};

void handleArguments(int argc, char *argv[], string &filePath)
{
    if (1 > argc || argc > 4)
    {
        cout << "Error: Invalid number of arguments!" << endl;
        exit(1);
    }

    string defaultFileName = "file.txt";
    string fileName = defaultFileName;
    string directory = filesystem::current_path().string();

    if (argc >= 2)
    {
        fileName = argv[1];
    }
    if (argc == 3)
    {
        directory = argv[2];
    }

    filePath = directory + "/" + fileName;

    if (!filesystem::exists(directory))
    {
        cout << "Directory does not exist: " << directory << endl;
        cout << "Creating directory: " << directory << endl;
        if (!filesystem::create_directories(directory))
        {
            cout << "Error: Failed to create directory: " << directory << endl;
            return;
        }
    }

    fstream file;

    if (filesystem::exists(filePath))
    {
        // If the file exists, open it
        file.open(filePath, ios::in | ios::out);
        if (!file)
        {
            cout << "Error: Unable to open file in read-write mode: " << filePath << endl;
            return;
        }
        cout << "File \"" << filePath << "\" opened successfully!" << endl;
    }
    else
    {
        // If the file doesn't exist, create it
        file.open(filePath, ios::out | ios::in | ios::app);
        if (!file)
        {
            cout << "Error: Unable to create file: " << filePath << endl;
            return;
        }
        cout << "File \"" << filePath << "\" created successfully!" << endl;
    }

    file.close();
}

int main(int argc, char *argv[])
{
    string filePath;

    handleArguments(argc, argv, filePath);

    LineEditor editor(filePath);
    editor.run();

    return 0;
}