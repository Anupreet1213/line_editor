#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

class LineEditor
{
private:
    vector<string> lines;
    string fileName;
    int choice, pos, wordPos;
    string newLine, newWord;
    string newContent;
    int cursorLine = 1;
    int cursorWord = 0;
    string tillWord;

public:
    void loadFile(string &fileName, vector<string> &lines)
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
            lines.push_back(line);
        }
        file.close();

        if (!lines.empty())
        {
            cursorLine = lines.size() + 1;
            cursorWord = 0;
            cout << "Cursor initialized at Line: " << cursorLine << ", Word: " << cursorWord << endl;
        }
    }

    void setCursorPosition(int line, int word)
    {
        cursorLine = line;
        cursorWord = word;
        cout << "Cursor moved to Line: " << cursorLine << ", Word: " << cursorWord << endl;
    }

    void displayLines(vector<string> &lines)
    {
        cout << endl;
        cout << "File Content: " << endl;
        for (int i = 0; i < lines.size(); i++)
        {
            cout << i + 1 << ". " << lines[i] << endl;
        }
    }

    void displaySpecificLine(vector<string> &lines, int lineNumber)
    {
        if (lineNumber < 1 || lineNumber > lines.size())
        {
            cout << "Error: Invalid line number.\n";
            return;
        }

        cout << "Line " << lineNumber << ": " << lines[lineNumber - 1] << endl;
    }

    void insertLine(vector<string> &lines, int &pos, string &newLine)
    {
        if (pos < 1 && pos > lines.size() + 1)
        {
            cout << "Error: Invalid position" << endl;
            return;
        }
        lines.insert(lines.begin() + (pos - 1), newLine);
        cursorLine = pos;
        cursorWord = 0;
        cout << "Line inserted at position " << pos << endl;
    }

    void insertWord(vector<string> &lines, string &newWord)
    {
        if (cursorLine < 1 || cursorLine > lines.size())
        {
            cout << "Inavlid cursor position for line." << endl;
            return;
        }
        string &line = lines[cursorLine - 1];
        stringstream ss(line);
        vector<string> words;
        string word;

        // Split line into words
        while (ss >> word)
        {
            words.push_back(word);
        }

        if (cursorWord < 0 || cursorWord > words.size())
        {
            cout << "Error: Invalid cursor position for word" << endl;
            return;
        }

        // Revisit
        words.insert(words.begin() + cursorWord, newWord);

        line = "";
        for (int i = 0; i < words.size(); i++)
        {
            line += words[i];
            if (i != words.size() - 1)
            {
                line += " ";
            }
        }

        cout << "Word inserted successfully!!" << endl;
    }

    void saveLine(string &fileName, vector<string> &lines)
    {
        ofstream file(fileName);
        if (!file.is_open())
        {
            cout << "Error: Unable to save file!!" << endl;
            return;
        }
        for (string &line : lines)
        {
            file << line << endl;
        }
        file.close();
        cout << "File saved successfully!!" << endl;
    }

    void deleteLine(vector<string> &lines, int &pos)
    {
        if (pos < 1 || pos > lines.size())
        {
            cout << "Invalid position!!" << endl;
            return;
        }
        lines.erase(lines.begin() + (pos - 1));
        cout << "Lines deleted successfully!!";
    }

    void deleteWord(vector<string> &lines, int lineIndex, int wordIndex)
    {
        if (lineIndex < 1 && lineIndex > lines.size())
        {
            cout << "Invalid line index.\n";
            return;
        }
        vector<string> words = splitWords(lines[lineIndex]);

        if (wordIndex >= 0 && wordIndex < words.size())
        {
            words.erase(words.begin() + wordIndex); // Remove the word
            lines[lineIndex] = joinWords(words);    // Update the line in the buffer
        }
        else
        {
            cout << "Invalid word index.\n";
        }
    }

    void modifyLine(vector<string> &lines, int pos, const string &newLine)
    {
        if (pos < 1 || pos > lines.size())
        {
            cout << "Error: Invalid position.\n";
            return;
        }
        lines[pos - 1] = newLine;
        cout << "Line modified successfully.\n";
    }

    // Revisit
    void modifyLineRange(vector<string> &lines, int cursorLine, int cursorWord, string &newContent, string till)
    {
        if (cursorLine < 1 || cursorLine > lines.size())
        {
            cout << "Error: Invalid cursor position for the line." << endl;
            return;
        }
        string &line = lines[cursorLine - 1];
        stringstream ss(line);
        vector<string> words;
        string word;

        // Split line into words
        while (ss >> word)
        {
            words.push_back(word);
        }

        if (cursorWord < 0 || cursorWord >= words.size())
        {
            cout << "Error: Inavlid cursor position for word." << endl;
            return;
        }

        int tillIndex = -1;

        // If index given
        if (isdigit(till[0]))
        {
            tillIndex = stoi(till);
            if (tillIndex < cursorWord || tillIndex >= words.size())
            {
                cout << "Error: Invalid index given." << endl;
                return;
            }
        }
        else
        {
            // If word given
            auto it = find(words.begin(), words.end(), till);
            if (it != words.end())
            {
                tillIndex = it - words.begin();
            }
            else
            {
                cout << "Error: Word not found." << endl;
                return;
            }
        }

        // Replace words from cursor to tillIndex with newContent
        vector<string> newWords;
        stringstream newLineStream(newLine);
        string newWord;
        while (newLineStream >> newWord)
        {
            newWords.push_back(newWord);
        }

        words.erase(words.begin() + cursorWord, words.begin() + tillIndex + 1);
        words.insert(words.begin() + cursorWord, newContent);

        line = "";
        for (int i = 0; i < words.size(); i++)
        {

            line += words[i];
            if (i != words.size() - 1)
            {
                line += " ";
            }
        }

        cout << "Line modified successfully!!" << endl;
    }

    void replaceWord(vector<string> &lines, int cursorLine, string &oldWord, string &newWord)
    {
        if (cursorLine < 1 || cursorLine > lines.size())
        {
            cout << "Error: Invalid cursor position for line." << endl;
            return;
        }
        string &line = lines[cursorLine - 1];
        int pos = line.find(oldWord);

        if (pos == -1)
        {
            cout << "Error: Word not found." << endl;
            return;
        }

        line.replace(pos, oldWord.length(), newWord);
        ;
        cout << "Word replaced successfully!!" << endl;
    }

    // Reuse
    vector<string> splitWords(string &line)
    {
        stringstream ss(line);
        string word;
        vector<string> words;

        while (ss >> word)
        {
            words.push_back(word);
        }

        return words;
    }

    string joinWords(vector<string> &words)
    {
        string result;
        for (int i = 0; i < words.size(); i++)
        {
            if (i > 0)
                result += " ";
            result += words[i];
        }
        return result;
    }

    void run()
    {
        cout << "Enter the filename" << endl;
        cin >> fileName;

        loadFile(fileName, lines);
        // displayLines(lines);

        if (lines.empty())
        {
            cout << "Can't open the file.\n";
            return;
        }

        do
        {
            cout << endl;
            cout << "1. Display file content" << endl;
            cout << "2. Display specific line" << endl;
            cout << "3. Insert line" << endl;
            cout << "4. Insert word" << endl;
            cout << "5. Move cursor" << endl;
            cout << "6. Delele line" << endl;
            cout << "7. Delete word" << endl;
            cout << "8. Modify line" << endl;
            cout << "9. Modify line in range" << endl;
            cout << "10. Replace a word" << endl;
            cout << "11. Save" << endl;
            cout << "12. Exit" << endl;
            cout << "Enter your choice" << endl;
            cin >> choice;

            switch (choice)
            {
            case 1:
                displayLines(lines);
                break;
            case 2:
                cout << "Enter line number to display: ";
                cin >> pos;
                displaySpecificLine(lines, pos);
                break;
            case 3:
                cout << "Enter new line";
                cin.ignore();
                getline(cin, newLine);
                insertLine(lines, cursorLine, newLine);
                cursorLine++;
                break;
            case 4:
                cout << "Enter new word: ";
                cin >> newWord;
                insertWord(lines, newWord);
                cursorWord++;
                break;
            case 5:
                cout << "Enter new cursor line: ";
                cin >> pos;
                cout << "Enter new cursor word (0 for line start): ";
                cin >> wordPos;
                setCursorPosition(pos, wordPos);
                break;
            case 6:
                cout << "Enter position to delete: ";
                cin >> pos;
                deleteLine(lines, pos);
                break;
            case 7:
                cout << "Enter word index to delete" << endl;
                cin >> cursorWord;
                deleteWord(lines, cursorLine, cursorWord);
                break;
            case 8:
                cout << "Enter line number to modify: ";
                cin >> pos;
                cin.ignore();
                cout << "Enter new line: ";
                getline(cin, newContent);
                modifyLine(lines, pos, newContent);
                break;
            case 9:
                cout << "Enter the starting cursor word position: ";
                cin >> cursorWord;
                cout << "Enter the ending word (or index) to modify to: ";
                cin >> tillWord;
                cout << "Enter new content for the line starting from cursor: \n";
                cin.ignore();
                getline(cin, newContent);
                modifyLineRange(lines, cursorLine, cursorWord, newContent, tillWord);
                break;
            case 10:
                cout << "Enter word to replace: " << endl;
                cin >> newWord;
                cout << "Enter new word: " << endl;
                cin >> newContent;
                replaceWord(lines, cursorLine, newWord, newContent);
                break;
            case 11:
                saveLine(fileName, lines);
                break;
            case 12:
                break;
            default:
                cout << "Invalid choice. Please try again" << endl;
            }
        } while (choice != 12);
    }
};

int main()
{
    LineEditor editor;
    editor.run();

    return 0;
}