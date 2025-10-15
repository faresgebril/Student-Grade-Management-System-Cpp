#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <sstream>

using namespace std;

// Base class for Person
class Person {
protected:
    string id;
    string name;
public:
    Person(string id, string name) : id(id), name(name) {}
    virtual ~Person() = default;
    string getId() const { return id; }
    string getName() const { return name; }
    virtual string serialize() const = 0;
    virtual void deserialize(const string& data) = 0;
};

// Grade class
class Grade {
private:
    string courseId;
    double score;
public:
    Grade(string courseId, double score) : courseId(courseId), score(score) {}
    string getCourseId() const { return courseId; }
    double getScore() const { return score; }
    string serialize() const { return courseId + "," + to_string(score); }
    static Grade deserialize(const string& data) {
        stringstream ss(data);
        string courseId;
        double score;
        getline(ss, courseId, ',');
        ss >> score;
        return Grade(courseId, score);
    }
};

// Course class
class Course : public Person {
public:
    Course(string courseId, string courseName) : Person(courseId, courseName) {}
    string serialize() const override { return id + "," + name; }
    void deserialize(const string& data) override {
        stringstream ss(data);
        getline(ss, id, ',');
        getline(ss, name);
    }
};

// Student class
class Student : public Person {
private:
    vector<Grade> grades;
public:
    Student(string studentId, string studentName) : Person(studentId, studentName) {}
    void addGrade(const Grade& grade) { grades.push_back(grade); }
    double calculateGPA() const {
        if (grades.empty()) return 0.0;
        double totalScore = 0.0;
        for (const Grade& grade : grades) {
            totalScore += grade.getScore();
        }
        return totalScore / grades.size();
    }
    string serialize() const override {
        stringstream ss;
        ss << id << "," << name << "|";
        for (size_t i = 0; i < grades.size(); ++i) {
            ss << grades[i].serialize();
            if (i < grades.size() - 1) ss << ";";
        }
        return ss.str();
    }
    void deserialize(const string& data) override {
        size_t pos = data.find("|");
        string info = data.substr(0, pos);
        string gradeData = data.substr(pos + 1);

        stringstream ss(info);
        getline(ss, id, ',');
        getline(ss, name);

        stringstream gradeStream(gradeData);
        string gradeStr;
        while (getline(gradeStream, gradeStr, ';')) {
            grades.push_back(Grade::deserialize(gradeStr));
        }
    }
};

// Function to save student and course data to file
void saveToFile(const vector<shared_ptr<Person>>& persons, const string& filename, const string& type) {
    ofstream file(filename, ios::out | ios::trunc);
    if (file.is_open()) {
        for (const auto& person : persons) {
            if ((type == "student" && dynamic_pointer_cast<Student>(person)) ||
                (type == "course" && dynamic_pointer_cast<Course>(person))) {
                file << person->serialize() << endl;
            }
        }
        file.close();
        cout << "Data saved to " << filename << " successfully." << endl;
    } else {
        cerr << "Error: Unable to open file " << filename << " for writing." << endl;
    }
}


// Function to load student and course data from file
void loadFromFile(vector<shared_ptr<Person>>& persons, const string& filename) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            if (filename == "students.txt") {
                auto student = make_shared<Student>("", "");
                student->deserialize(line);
                persons.push_back(student);
            } else if (filename == "courses.txt") {
                auto course = make_shared<Course>("", "");
                course->deserialize(line);
                persons.push_back(course);
            }
        }
        file.close();
        cout << "Data loaded from " << filename << " successfully." << endl;
    } else {
        cerr << "Error: Unable to open file " << filename << " for reading." << endl;
    }
}

// Function to add a new student
void addStudent(vector<shared_ptr<Person>>& persons, const shared_ptr<Student>& student) {
    persons.push_back(student);
    saveToFile(persons, "students.txt", "student"); // Save only student data to file after adding
    cout << "Student added successfully." << endl;
}

// Function to add a new course
void addCourse(vector<shared_ptr<Person>>& persons, const shared_ptr<Course>& course) {
    persons.push_back(course);
    saveToFile(persons, "courses.txt", "course"); // Save only course data to file after adding
    cout << "Course added successfully." << endl;
}

// Function to record a grade for a student
void recordGrade(vector<shared_ptr<Person>>& persons, const string& studentId, const string& courseId, double score) {
    for (const auto& person : persons) {
        if (person->getId() == studentId) {
            auto student = dynamic_pointer_cast<Student>(person);
            if (student) {
                student->addGrade(Grade(courseId, score));
                saveToFile(persons, "students.txt", "student"); // Save only student data to file after recording grade
                cout << "Grade recorded successfully." << endl;
                return;
            }
        }
    }
    cerr << "Error: Student with ID " << studentId << " not found." << endl;
}

// Function to generate a grade report for all students
void generateGradeReport(const vector<shared_ptr<Person>>& persons) {
    cout << "Grade Report\n";
    for (const auto& person : persons) {
        auto student = dynamic_pointer_cast<Student>(person);
        if (student) {
            cout << "Student: " << student->getName() << ", GPA: " << student->calculateGPA() << endl;
        }
    }
}

int main() {
    vector<shared_ptr<Person>> persons;
    loadFromFile(persons, "students.txt");
    loadFromFile(persons, "courses.txt");

    int choice;
    do {
        cout << "1. Add Student\n";
        cout << "2. Add Course\n";
        cout << "3. Record Grades\n";
        cout << "4. Calculate GPA\n";
        cout << "5. Generate Grade Report\n";
        cout << "6. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                string studentId, studentName;
                cout << "Enter student ID: ";
                cin >> studentId;
                cout << "Enter student name: ";
                cin.ignore();
                getline(cin, studentName);
                addStudent(persons, make_shared<Student>(studentId, studentName));
                break;
            }
            case 2: {
                string courseId, courseName;
                cout << "Enter course ID: ";
                cin >> courseId;
                cout << "Enter course name: ";
                cin.ignore();
                getline(cin, courseName);
                addCourse(persons, make_shared<Course>(courseId, courseName));
                break;
            }
            case 3: {
                string studentId, courseId;
                double score;
                cout << "Enter student ID: ";
                cin >> studentId;
                cout << "Enter course ID: ";
                cin >> courseId;
                cout << "Enter score: ";
                cin >> score;
                recordGrade(persons, studentId, courseId, score);
                break;
            }
            case 4: {
                string studentId;
                cout << "Enter student ID: ";
                cin >> studentId;
                for (const auto& person : persons) {
                    if (person->getId() == studentId) {
                        auto student = dynamic_pointer_cast<Student>(person);
                        if (student) {
                            cout << "GPA for student " << student->getName() << ": " << student->calculateGPA() << endl;
                            break;
                        }
                    }
                }
                break;
            }
            case 5: {
                generateGradeReport(persons);
                break;
            }
            case 6: {
                cout << "Exiting...\n";
                break;
            }
            default: {
                cout << "Invalid choice. Please try again.\n";
                break;
            }
        }
    } while (choice != 6);

    return 0;
}
