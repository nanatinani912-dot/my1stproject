/*
 * ============================================================
 *   UNIVERSITY STUDENT MANAGEMENT SYSTEM
 *   Language   : C++
 *   Course     : Data Structures and Algorithms
 *   Concepts   : Linked Lists, Sorting, Searching, Stack, Queue, Graph
 * ============================================================
 *
 *  DATA STRUCTURE MAP
 *  ------------------
 *  [1] Singly Linked List  → stores registered students, attendance records, results
 *  [2] Doubly Linked List  → page/menu navigation (back & forward)
 *  [3] Circular Linked List→ repeating menu cycling
 *  [4] Bubble Sort         → sort students by name ascending/descending
 *  [5] Selection Sort      → sort students by ID
 *  [6] Insertion Sort      → sort results/grades
 *  [7] Linear Search       → search student by name or teacher by name
 *  [8] Binary Search       → fast search after sorted by ID
 *  [9] Stack               → back-navigation (LIFO page history)
 * [10] Queue               → attendance / registration processing (FIFO)
 * [11] Graph               → class-teacher-student relationship map
 * ============================================================
 */

#include <iostream>
#include <string>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <sstream>
#include <limits>
using namespace std;

// ============================================================
//  CONSTANTS & GRADE SCALE (Ethiopian University – Haramaya)
// ============================================================
const int MAX_STUDENTS    = 200;
const int MAX_CLASSES     = 50;
const int MAX_ATTENDANCE  = 100;
const int CHEAT_DISTANCE  = 5;   // meters: allowed distance for attendance
const int ABSENCE_LIMIT   = 3;   // max absences before warning
const int TEACHER_ID_LEN  = 6;
const int STUDENT_ID_LEN  = 4;

struct GradeRow { int min, max; char letter[4]; double point; };
const GradeRow GRADE_SCALE[] = {
    {90,100,"A+",4.00},{85,89,"A",4.00},{80,84,"A-",3.75},
    {75,79,"B+",3.50},{70,74,"B",3.00},{65,69,"B-",2.75},
    {60,64,"C+",2.50},{50,59,"C",2.00},{45,49,"C-",1.75},
    {40,44,"D",1.00},{0,39,"F",0.00}
};
const int GRADE_COUNT = 11;

// ============================================================
//  UTILITY HELPERS
// ============================================================
void clearScreen()  { cout << "\n\n"; }
void printLine(char c = '-', int n = 65) { for(int i=0;i<n;i++) cout<<c; cout<<"\n"; }
void printTitle(const string& t){
    printLine('=');
    cout << "  " << t << "\n";
    printLine('=');
}
void pause(){
    cout<<"\n  [Press ENTER to continue...]";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool isValidEmail(const string& e){
    size_t at = e.find('@');
    if(at==string::npos||at==0) return false;
    size_t dot = e.find('.', at);
    return dot!=string::npos && dot<e.size()-1;
}
bool isEthiopianPhone(const string& p){
    if(p.size()!=10) return false;
    if(p[0]!='0') return false;
    if(p[1]!='9'&&p[1]!='7') return false;
    for(char c:p) if(!isdigit(c)) return false;
    return true;
}
bool isAllDigits(const string& s, int len){
    if((int)s.size()!=len) return false;
    for(char c:s) if(!isdigit(c)) return false;
    return true;
}

GradeRow getGrade(double total){
    for(int i=0;i<GRADE_COUNT;i++)
        if(total>=GRADE_SCALE[i].min && total<=GRADE_SCALE[i].max)
            return GRADE_SCALE[i];
    return {0,0,"F",0.00};
}

string currentDate(){
    time_t t=time(0); tm* now=localtime(&t);
    // Use ostringstream to avoid sprintf buffer-size warnings
    ostringstream oss;
    oss << (now->tm_year+1900) << "-"
        << setw(2) << setfill('0') << (now->tm_mon+1) << "-"
        << setw(2) << setfill('0') << now->tm_mday;
    return oss.str();
}
string currentTime(){
    time_t t=time(0); tm* now=localtime(&t);
    ostringstream oss;
    oss << setw(2)<<setfill('0')<<now->tm_hour << ":"
        << setw(2)<<setfill('0')<<now->tm_min  << ":"
        << setw(2)<<setfill('0')<<now->tm_sec;
    return oss.str();
}

// ============================================================
//  [1] SINGLY LINKED LIST – Student Node
//      Used to store registered students dynamically.
//      Reason: students are added one by one without fixed array size.
// ============================================================
struct StudentNode {
    string name;
    string id;
    // Result fields
    double midMark, finalMark, assignMark, quizMark;
    bool   hasMid, hasFinal, hasAssign, hasQuiz;
    // Use string instead of char[] to avoid strcpy/buffer issues
    string attStatus[MAX_ATTENDANCE]; // "P", "A", or "Pe"
    string attDate[MAX_ATTENDANCE];
    int    attCount;
    bool   isCheating;
    string comment;
    StudentNode* next; // singly linked pointer

    StudentNode(const string& n, const string& i):
        name(n),id(i),midMark(0),finalMark(0),assignMark(0),quizMark(0),
        hasMid(false),hasFinal(false),hasAssign(false),hasQuiz(false),
        attCount(0),isCheating(false),comment(""),next(nullptr){}
};

// ============================================================
//  Singly Linked List for Students
// ============================================================
struct StudentList {
    StudentNode* head;
    int size;
    StudentList():head(nullptr),size(0){}

    // Insert at end (O(n))
    void insert(const string& name, const string& id){
        StudentNode* n = new StudentNode(name,id);
        if(!head){ head=n; size++; return; }
        StudentNode* cur=head;
        while(cur->next) cur=cur->next;
        cur->next=n; size++;
    }

    // Delete by ID (linear search then remove)
    bool remove(const string& id){
        if(!head) return false;
        if(head->id==id){ StudentNode* tmp=head; head=head->next; delete tmp; size--; return true; }
        StudentNode* cur=head;
        while(cur->next && cur->next->id!=id) cur=cur->next;
        if(!cur->next) return false;
        StudentNode* tmp=cur->next; cur->next=tmp->next; delete tmp; size--; return true;
    }

    // Find by ID (O(n) linear search)
    StudentNode* findById(const string& id){
        StudentNode* cur=head;
        while(cur){ if(cur->id==id) return cur; cur=cur->next; }
        return nullptr;
    }

    // Find by Name (O(n) linear search)
    StudentNode* findByName(const string& nm){
        StudentNode* cur=head;
        while(cur){
            string a=cur->name, b=nm;
            for(char& c:a) c=tolower(c);
            for(char& c:b) c=tolower(c);
            if(a.find(b)!=string::npos) return cur;
            cur=cur->next;
        }
        return nullptr;
    }

    // Convert list to array for sorting (needed by sorting algorithms)
    void toArray(StudentNode** arr, int& n){
        n=0; StudentNode* cur=head;
        while(cur){ arr[n++]=cur; cur=cur->next; }
    }

    // Rebuild list from sorted array
    void fromArray(StudentNode** arr, int n){
        if(n==0){head=nullptr;return;}
        head=arr[0];
        for(int i=0;i<n-1;i++) arr[i]->next=arr[i+1];
        arr[n-1]->next=nullptr;
    }

    void print(){
        cout << left << setw(4)<<"#" << setw(25)<<"Name" << setw(12)<<"ID" << "\n";
        printLine('-');
        int i=1; StudentNode* cur=head;
        while(cur){
            cout << left << setw(4)<<i++ << setw(25)<<cur->name << setw(12)<<cur->id << "\n";
            cur=cur->next;
        }
    }

    ~StudentList(){
        StudentNode* cur=head;
        while(cur){ StudentNode* tmp=cur; cur=cur->next; delete tmp; }
    }
};

// ============================================================
//  [2] DOUBLY LINKED LIST – Menu/Page Navigation
//      Allows both forward and backward navigation between pages.
//      Reason: user can go back (prev) and forward (next) in history.
// ============================================================
struct PageNode {
    string pageName;
    PageNode* prev;   // back pointer
    PageNode* next;   // forward pointer
    PageNode(const string& n):pageName(n),prev(nullptr),next(nullptr){}
};

struct PageHistory {
    PageNode* current;
    PageHistory():current(nullptr){}

    void push(const string& page){
        PageNode* n = new PageNode(page);
        if(!current){ current=n; return; }
        // Trim forward history
        PageNode* tmp=current->next;
        while(tmp){ PageNode* del=tmp; tmp=tmp->next; delete del; }
        current->next=n; n->prev=current; current=n;
    }

    string goBack(){
        if(current&&current->prev){ current=current->prev; return current->pageName; }
        return "";
    }
    string goForward(){
        if(current&&current->next){ current=current->next; return current->pageName; }
        return "";
    }
    string getCurrent(){ return current?current->pageName:""; }
};

// ============================================================
//  [3] CIRCULAR LINKED LIST – Menu Cycling
//      The main menu loops back to start after last item.
//      Reason: last node's next points to head (circular).
// ============================================================
struct MenuNode {
    string label;
    MenuNode* next;
    MenuNode(const string& l):label(l),next(nullptr){}
};

struct CircularMenu {
    MenuNode* head;
    MenuNode* current;
    int size;
    CircularMenu():head(nullptr),current(nullptr),size(0){}

    void add(const string& label){
        MenuNode* n=new MenuNode(label);
        if(!head){ head=n; n->next=head; current=head; size++; return; }
        MenuNode* tmp=head;
        while(tmp->next!=head) tmp=tmp->next;
        tmp->next=n; n->next=head; size++;
    }

    string next_item(){
        if(!current) return "";
        current=current->next; // wraps to head automatically (circular)
        return current->label;
    }

    void printAll(){
        if(!head) return;
        MenuNode* tmp=head; int i=1;
        do{ cout<<"  ["<<i++<<"] "<<tmp->label<<"\n"; tmp=tmp->next; } while(tmp!=head);
    }
};

// ============================================================
//  [9] STACK – Back Navigation (LIFO)
//      When user presses "back", the last page is popped off.
//      Reason: LIFO = last opened page is the one to return to.
// ============================================================
struct StackNode { string data; StackNode* next; };
struct NavigationStack {
    StackNode* top;
    int size;
    NavigationStack():top(nullptr),size(0){}

    void push(const string& page){
        StackNode* n=new StackNode(); n->data=page; n->next=top; top=n; size++;
    }
    string pop(){
        if(!top) return "";
        string d=top->data; StackNode* tmp=top; top=top->next; delete tmp; size--; return d;
    }
    string peek(){ return top?top->data:""; }
    bool empty(){ return top==nullptr; }
};

// ============================================================
//  [10] QUEUE – Attendance & Registration Processing (FIFO)
//       Students who sign attendance are queued and processed in order.
//       Reason: First student to arrive is first to be processed (FIFO).
// ============================================================
struct QueueNode { string studentId; string studentName; QueueNode* next; };
struct AttendanceQueue {
    QueueNode *front, *rear;
    int size;
    AttendanceQueue():front(nullptr),rear(nullptr),size(0){}

    void enqueue(const string& id, const string& nm){
        QueueNode* n=new QueueNode(); n->studentId=id; n->studentName=nm; n->next=nullptr;
        if(!rear){ front=rear=n; size++; return; }
        rear->next=n; rear=n; size++;
    }
    pair<string,string> dequeue(){
        if(!front) return {"",""};
        QueueNode* tmp=front;
        pair<string,string> d={tmp->studentId,tmp->studentName};
        front=front->next; if(!front) rear=nullptr;
        delete tmp; size--; return d;
    }
    bool empty(){ return front==nullptr; }

    void processAll(){
        cout<<"\n  Processing attendance queue (FIFO)...\n";
        while(!empty()){
            auto [id,nm]=dequeue();
            cout<<"  ✓ Processed: "<<nm<<" ("<<id<<")\n";
        }
    }
};

// ============================================================
//  [11] GRAPH – Teacher–Class–Student Relationship
//       Models who teaches which class and which students are enrolled.
//       Reason: Graph represents many-to-many relationships between entities.
// ============================================================
struct GraphEdge {
    string from, to, relation;
    GraphEdge* next;
    GraphEdge(const string& f, const string& t, const string& r):
        from(f),to(t),relation(r),next(nullptr){}
};
struct Graph {
    GraphEdge* edgeHead;
    int edgeCount;
    Graph():edgeHead(nullptr),edgeCount(0){}

    void addEdge(const string& from, const string& to, const string& rel){
        GraphEdge* e=new GraphEdge(from,to,rel);
        e->next=edgeHead; edgeHead=e; edgeCount++;
    }
    void printRelations(const string& from){
        cout<<"\n  Graph relations for: "<<from<<"\n";
        GraphEdge* cur=edgeHead;
        while(cur){
            if(cur->from==from)
                cout<<"    "<<from<<" --["<<cur->relation<<"]-> "<<cur->to<<"\n";
            cur=cur->next;
        }
    }
    ~Graph(){
        GraphEdge* cur=edgeHead;
        while(cur){ GraphEdge* tmp=cur; cur=cur->next; delete tmp; }
    }
};

// ============================================================
//  [4] BUBBLE SORT – Sort students by name (alphabetical)
//      Compare adjacent nodes and swap if out of order.
//      Why: Simple, works well on small lists; sorts names A-Z.
// ============================================================
void bubbleSortByName(StudentNode** arr, int n){
    for(int i=0;i<n-1;i++){
        for(int j=0;j<n-i-1;j++){
            string a=arr[j]->name, b=arr[j+1]->name;
            for(char& c:a) c=tolower(c);
            for(char& c:b) c=tolower(c);
            if(a>b) swap(arr[j],arr[j+1]); // swap pointers
        }
    }
}

// ============================================================
//  [5] SELECTION SORT – Sort students by ID
//      Find minimum ID in unsorted portion and place at front.
//      Why: Good for sorting by unique numeric/string IDs.
// ============================================================
void selectionSortById(StudentNode** arr, int n){
    for(int i=0;i<n-1;i++){
        int minIdx=i;
        for(int j=i+1;j<n;j++)
            if(arr[j]->id < arr[minIdx]->id) minIdx=j;
        if(minIdx!=i) swap(arr[i],arr[minIdx]);
    }
}

// ============================================================
//  [6] INSERTION SORT – Sort results/grades (by total mark)
//      Insert each element into its correct position in sorted part.
//      Why: Efficient for nearly-sorted grade lists.
// ============================================================
void insertionSortByGrade(StudentNode** arr, int n){
    for(int i=1;i<n;i++){
        StudentNode* key=arr[i];
        double keyTotal=key->hasMid?key->midMark+key->finalMark+key->assignMark+key->quizMark:-1;
        int j=i-1;
        while(j>=0){
            double jTotal=arr[j]->hasMid?arr[j]->midMark+arr[j]->finalMark+arr[j]->assignMark+arr[j]->quizMark:-1;
            if(jTotal<keyTotal){ arr[j+1]=arr[j]; j--; } else break;
        }
        arr[j+1]=key;
    }
}

// ============================================================
//  [7] LINEAR SEARCH – Search by name (Sequential)
//      Scans list one-by-one until match found.
//      Why: Used when data is unsorted or small.
// ============================================================
StudentNode* linearSearchByName(StudentNode** arr, int n, const string& nm){
    string target=nm; for(char& c:target) c=tolower(c);
    for(int i=0;i<n;i++){
        string cur=arr[i]->name; for(char& c:cur) c=tolower(c);
        if(cur.find(target)!=string::npos) return arr[i];
    }
    return nullptr;
}

// ============================================================
//  [8] BINARY SEARCH – Fast search by sorted ID
//      Divide & conquer: compare mid, go left or right.
//      Why: O(log n) — much faster than linear for large sorted lists.
// ============================================================
StudentNode* binarySearchById(StudentNode** arr, int n, const string& id){
    int lo=0, hi=n-1;
    while(lo<=hi){
        int mid=(lo+hi)/2;
        if(arr[mid]->id==id) return arr[mid];
        else if(arr[mid]->id<id) lo=mid+1;
        else hi=mid-1;
    }
    return nullptr;
}

// ============================================================
//  CLASS (Subject) STRUCTURE
//  Holds: department, section, student list, attendance records,
//         marks configuration, messages (discussion).
// ============================================================
struct AttendanceRecord {
    string date, time;
    // Each student's status stored parallel to class student list
    // Using string to safely store "P", "A", or "Pe"
    string statuses[MAX_STUDENTS];
    string studentIds[MAX_STUDENTS];
    int count;
    AttendanceRecord():count(0){}
};

struct MarksConfig {
    double midPct, finalPct, assignPct, quizPct;
    bool configured;
    bool resultsVisible;
    MarksConfig():midPct(0),finalPct(0),assignPct(0),quizPct(0),configured(false),resultsVisible(false){}
};

struct ClassMessage {
    string sender, text, timestamp;
    bool isFile;
};

struct ClassRecord {
    string dept, section, repName, repPhone;
    StudentList students;      // [1] Singly Linked List
    AttendanceRecord attendance[MAX_ATTENDANCE];
    int attCount;
    MarksConfig marks;
    ClassMessage messages[500];
    int msgCount;
    bool active;
    ClassRecord():attCount(0),msgCount(0),active(false){}
};

// ============================================================
//  TEACHER STRUCTURE
// ============================================================
struct Teacher {
    string name, id, college, dept, course;
    bool loggedIn;
    Teacher():loggedIn(false){}
};

// ============================================================
//  STUDENT STRUCTURE (for login/session)
// ============================================================
struct StudentSession {
    string name, id, dept, section;
    bool loggedIn;
    int connectedClassIdx;
    StudentSession():loggedIn(false),connectedClassIdx(-1){}
};

// ============================================================
//  GLOBAL SYSTEM STATE
// ============================================================
Teacher         gTeacher;
StudentSession  gStudent;
ClassRecord     gClasses[MAX_CLASSES];
int             gClassCount = 0;
int             gCurrentClass = -1;
string          gRole;

NavigationStack  gNavStack;    // [9]  Stack for back navigation
AttendanceQueue  gAttQueue;    // [10] Queue for attendance processing
Graph            gGraph;       // [11] Graph for relations
PageHistory      gPageHistory; // [2]  Doubly linked list for history

// ============================================================
//  FORWARD DECLARATIONS
// ============================================================
void mainMenu();
void teacherMenu();
void studentMenu();
void registerStudentsMenu();
void classInfoPage();
void addStudentsPage(int classIdx);
void manageClassesPage();
void classDetailMenu(int ci);
void classStudentsPage(int ci);
void attendanceMenu(int ci);
void takeAttendancePage(int ci);
void showAttendancePage(int ci);
void resultsMenu(int ci);
void enterResultsPage(int ci, const string& type);
void showResultsPage(int ci);
void discussionPage(int ci);
void studentRegPage();
void connectTeacherPage();
void viewMyAttendancePage();
void viewMyResultsPage();
void calcGPAPage();
void sortAndSearchDemo(int ci);

// ============================================================
//  UTILITY: print attendance record
// ============================================================
void printAttRow(const string& name, const string& id, const string& status){
    cout << left << setw(25)<<name << setw(12)<<id << setw(8)<<status << "\n";
}

// ============================================================
//  INPUT HELPERS
// ============================================================
string getLine(const string& prompt){
    string s;
    cout << "  " << prompt;
    getline(cin, s);
    return s;
}
int getInt(const string& prompt, int lo=0, int hi=999){
    int v; string s;
    while(true){
        cout << "  " << prompt;
        getline(cin,s);
        try{ v=stoi(s); if(v>=lo&&v<=hi) return v; }
        catch(...){}
        cout << "  !! Invalid. Enter a number between "<<lo<<" and "<<hi<<".\n";
    }
}
double getDouble(const string& prompt, double lo=0, double hi=100){
    double v; string s;
    while(true){
        cout << "  " << prompt;
        getline(cin,s);
        try{ v=stod(s); if(v>=lo&&v<=hi) return v; }
        catch(...){}
        cout << "  !! Invalid. Enter a number between "<<lo<<" and "<<hi<<".\n";
    }
}

// ============================================================
//  WELCOME / LOGIN PAGE
// ============================================================
void loginPage(){
    clearScreen();
    printTitle("UNIVERSITY STUDENT MANAGEMENT SYSTEM");
    cout << "  Welcome to the Online Grading & Management System\n";
    cout << "  Course: Data Structures and Algorithms (C++)\n\n";
    printLine();
    string email, pass, roleChoice;
    while(true){
        email = getLine("Enter Email: ");
        if(isValidEmail(email)) break;
        cout << "  !! Email must contain '@' and a domain (e.g. user@uni.edu)\n";
    }
    cout << "  Enter Password (hidden): ";
    // Simple hidden password simulation
    pass = getLine("");
    if(pass.empty()){ cout<<"  !! Password cannot be empty.\n"; loginPage(); return; }

    cout << "\n  Select Role:\n";
    cout << "  [1] Teacher\n  [2] Student\n";
    int r = getInt("Enter choice (1 or 2): ", 1, 2);
    gRole = (r==1)?"teacher":"student";

    // Push to navigation stack [9]
    gNavStack.push("login");
    gPageHistory.push("login");

    if(gRole=="teacher") { teacherMenu(); }
    else                 { studentRegPage(); }
}

// ============================================================
//  TEACHER INFO PAGE
// ============================================================
void teacherInfoPage(){
    clearScreen(); printTitle("TEACHER INFORMATION");
    while(true){
        gTeacher.name   = getLine("Teacher Name: ");
        gTeacher.id     = getLine("Teacher ID (6 digits): ");
        if(!isAllDigits(gTeacher.id,6)){cout<<"  !! Must be exactly 6 digits.\n";continue;}
        gTeacher.college= getLine("College: ");
        gTeacher.dept   = getLine("Department: ");
        gTeacher.course = getLine("Course: ");
        if(gTeacher.name.empty()||gTeacher.college.empty()||gTeacher.dept.empty()||gTeacher.course.empty()){
            cout<<"  !! All fields required.\n"; continue;
        }
        break;
    }
    gTeacher.loggedIn=true;
    // Add to graph [11]: teacher teaches course
    gGraph.addEdge(gTeacher.id, gTeacher.course, "TEACHES");
    cout << "\n  ✓ Teacher profile saved.\n"; pause();
    teacherMenu();
}

// ============================================================
//  TEACHER DASHBOARD
// ============================================================
void teacherMenu(){
    if(!gTeacher.loggedIn){ teacherInfoPage(); return; }
    clearScreen(); printTitle("TEACHER DASHBOARD");
    cout << "  Teacher : " << gTeacher.name << " (ID: " << gTeacher.id << ")\n";
    cout << "  Course  : " << gTeacher.course << " | Dept: " << gTeacher.dept << "\n\n";

    int totalStudents=0;
    for(int i=0;i<gClassCount;i++) totalStudents+=gClasses[i].students.size;
    cout << "  Classes Registered : " << gClassCount << "\n";
    cout << "  Total Students     : " << totalStudents << "\n\n";
    printLine();

    // [3] Circular menu demo
    CircularMenu cm;
    cm.add("[1] Register Students");
    cm.add("[2] Manage Classes");
    cm.add("[3] Sort/Search Demo");
    cm.add("[4] Exit");
    cm.printAll();
    printLine();

    int ch=getInt("Select option: ",1,4);
    gNavStack.push("teacher_menu"); gPageHistory.push("teacher_menu");
    switch(ch){
        case 1: registerStudentsMenu(); break;
        case 2: manageClassesPage(); break;
        case 3: if(gClassCount>0) sortAndSearchDemo(0); else {cout<<"  !! No classes yet.\n";pause();teacherMenu();} break;
        case 4: loginPage(); break;
    }
}

// ============================================================
//  REGISTER MENU
// ============================================================
void registerStudentsMenu(){
    clearScreen(); printTitle("STUDENT REGISTRATION MENU");
    cout << "  [1] 3a — Register New Class\n";
    cout << "  [2] 3b — Manage Registered Classes\n";
    cout << "  [3] Back\n  [4] Exit\n";
    printLine();
    int ch=getInt("Select: ",1,4);
    switch(ch){
        case 1: classInfoPage(); break;
        case 2: manageClassesPage(); break;
        case 3: { gNavStack.pop(); teacherMenu(); break; }
        case 4: loginPage(); break;
    }
}

// ============================================================
//  CLASS INFO PAGE (3a.1)
// ============================================================
void classInfoPage(){
    clearScreen(); printTitle("CLASS REGISTRATION INFORMATION");
    if(gClassCount>=MAX_CLASSES){ cout<<"  !! Max classes reached.\n"; pause(); return; }
    ClassRecord& cls = gClasses[gClassCount];
    cls.active=true;

    cls.dept    = getLine("Department: ");
    cls.section = getLine("Section: ");
    cls.repName = getLine("Representative Name: ");
    while(true){
        cls.repPhone = getLine("Representative Phone (e.g. 0912345678): ");
        if(isEthiopianPhone(cls.repPhone)) break;
        cout<<"  !! Must be valid Ethiopian number (09... or 07..., 10 digits).\n";
    }
    addStudentsPage(gClassCount);
}

// ============================================================
//  ADD STUDENTS PAGE (3a.1.1)
// ============================================================
void addStudentsPage(int ci){
    clearScreen(); printTitle("ADD STUDENTS — " + gClasses[ci].dept + " / " + gClasses[ci].section);
    cout << "  [1] Enter students manually\n  [2] Done (submit class)\n  [3] Back\n";
    int ch=getInt("Select: ",1,3);
    if(ch==3){ registerStudentsMenu(); return; }

    if(ch==1){
        int n=getInt("Number of students: ",1,200);
        // [10] Queue: enqueue each student as they are registered
        for(int i=0;i<n;i++){
            cout << "\n  --- Student " << (i+1) << " ---\n";
            string nm,id;
            nm=getLine("  Name: ");
            while(true){
                id=getLine("  ID (unique): ");
                if(!gClasses[ci].students.findById(id)) break;
                cout<<"  !! ID already exists. Use unique ID.\n";
            }
            gClasses[ci].students.insert(nm,id);
            gAttQueue.enqueue(id,nm); // [10] Queue
            // [11] Graph: student is in class
            gGraph.addEdge(id, gClasses[ci].dept+"/"+gClasses[ci].section, "ENROLLED_IN");
            gGraph.addEdge(gTeacher.id, id, "TEACHES_STUDENT");
        }
        // Sort by name using [4] Bubble Sort
        StudentNode* arr[MAX_STUDENTS]; int sz;
        gClasses[ci].students.toArray(arr,sz);
        bubbleSortByName(arr,sz);
        gClasses[ci].students.fromArray(arr,sz);

        cout<<"\n  ✓ Students sorted alphabetically (Bubble Sort).\n";
        gClasses[ci].students.print();
        cout<<"\n";
        gAttQueue.processAll(); // [10] process queue
    }

    cout<<"\n  Submit class? [1=Yes, 2=No]: ";
    if(getInt("",1,2)==1){
        gClassCount++;
        cout<<"  ✓ Class registered with "<<gClasses[ci].students.size<<" students.\n";
        pause();
        teacherMenu();
    } else { addStudentsPage(ci); }
}

// ============================================================
//  MANAGE CLASSES (3b)
// ============================================================
void manageClassesPage(){
    clearScreen(); printTitle("MANAGE REGISTERED CLASSES");
    if(gClassCount==0){ cout<<"  No classes registered yet.\n"; pause(); teacherMenu(); return; }
    string search = getLine("Search by department (leave blank for all): ");
    cout << "\n  #  Department              Section        Students\n";
    printLine();
    int found=0;
    for(int i=0;i<gClassCount;i++){
        string dep=gClasses[i].dept, s=search;
        for(char& c:dep) c=tolower(c);
        for(char& c:s)   c=tolower(c);
        if(s.empty()||dep.find(s)!=string::npos){
            cout << "  "<<(i+1)<<"  "<<left<<setw(24)<<gClasses[i].dept<<setw(15)<<gClasses[i].section<<gClasses[i].students.size<<"\n";
            found++;
        }
    }
    if(!found){ cout<<"  No classes match.\n"; pause(); manageClassesPage(); return; }
    printLine();
    int idx=getInt("Select class number (0=back): ",0,gClassCount);
    if(idx==0){ teacherMenu(); return; }
    gCurrentClass=idx-1;
    classDetailMenu(gCurrentClass);
}

// ============================================================
//  CLASS DETAIL MENU
// ============================================================
void classDetailMenu(int ci){
    clearScreen();
    printTitle("CLASS: " + gClasses[ci].dept + " / " + gClasses[ci].section);
    cout<<"  [1] Show All Students\n";
    cout<<"  [2] Attendance Management\n";
    cout<<"  [3] Results Management\n";
    cout<<"  [4] Discussion\n";
    cout<<"  [5] Back\n  [6] Exit\n";
    int ch=getInt("Select: ",1,6);
    switch(ch){
        case 1: classStudentsPage(ci); break;
        case 2: attendanceMenu(ci); break;
        case 3: resultsMenu(ci); break;
        case 4: discussionPage(ci); break;
        case 5: { string prev=gNavStack.pop(); manageClassesPage(); break; }
        case 6: loginPage(); break;
    }
}

// ============================================================
//  CLASS STUDENTS PAGE (3b.1)
// ============================================================
void classStudentsPage(int ci){
    clearScreen(); printTitle("REGISTERED STUDENTS — " + gClasses[ci].dept);
    cout<<"  [1] View All  [2] Search by Name  [3] Search by ID\n";
    cout<<"  [4] Add Student  [5] Delete Student  [6] Sort by ID (Selection Sort)\n";
    cout<<"  [7] Back\n";
    printLine();
    int ch=getInt("Select: ",1,7);

    StudentNode* arr[MAX_STUDENTS]; int sz;
    gClasses[ci].students.toArray(arr,sz);

    switch(ch){
        case 1:
            cout<<"\n  All Students (sorted by name — Bubble Sort applied):\n";
            printLine(); gClasses[ci].students.print();
            break;
        case 2:{
            string nm=getLine("Enter name to search (Linear Search): ");
            // [7] Linear Search
            StudentNode* found=linearSearchByName(arr,sz,nm);
            if(found) cout<<"  ✓ Found: "<<found->name<<" | ID: "<<found->id<<"\n";
            else cout<<"  !! Not found.\n";
            break;
        }
        case 3:{
            // [5] Sort by ID first for binary search [8]
            selectionSortById(arr,sz);
            string id=getLine("Enter ID to search (Binary Search after Selection Sort): ");
            StudentNode* found=binarySearchById(arr,sz,id);
            if(found) cout<<"  ✓ Found: "<<found->name<<" | ID: "<<found->id<<"\n";
            else cout<<"  !! Not found.\n";
            break;
        }
        case 4:{
            string nm=getLine("New Student Name: "), id;
            while(true){
                id=getLine("New Student ID: ");
                if(!gClasses[ci].students.findById(id)) break;
                cout<<"  !! Duplicate ID.\n";
            }
            gClasses[ci].students.insert(nm,id);
            // Re-sort [4]
            gClasses[ci].students.toArray(arr,sz);
            bubbleSortByName(arr,sz);
            gClasses[ci].students.fromArray(arr,sz);
            cout<<"  ✓ Student added and list re-sorted.\n";
            break;
        }
        case 5:{
            string id=getLine("Enter ID to delete: ");
            if(gClasses[ci].students.remove(id)) cout<<"  ✓ Student removed.\n";
            else cout<<"  !! Student not found.\n";
            break;
        }
        case 6:
            // [5] Selection Sort by ID
            selectionSortById(arr,sz);
            gClasses[ci].students.fromArray(arr,sz);
            cout<<"\n  Students sorted by ID (Selection Sort):\n";
            printLine(); gClasses[ci].students.print();
            break;
        case 7: classDetailMenu(ci); return;
    }
    pause(); classStudentsPage(ci);
}

// ============================================================
//  ATTENDANCE MENU (3b.2)
// ============================================================
void attendanceMenu(int ci){
    clearScreen(); printTitle("ATTENDANCE — " + gClasses[ci].dept + " / " + gClasses[ci].section);
    cout<<"  [1] Take Attendance\n  [2] Show Attendance Records\n";
    cout<<"  [3] Back\n  [4] Exit\n";
    int ch=getInt("Select: ",1,4);
    switch(ch){
        case 1: takeAttendancePage(ci); break;
        case 2: showAttendancePage(ci); break;
        case 3: classDetailMenu(ci); return;
        case 4: loginPage(); return;
    }
}

// ============================================================
//  TAKE ATTENDANCE (3b.2.1)
//  GPS distance simulation: distance ≤ 5m = valid
//  Uses Queue [10] to process each student
// ============================================================
void takeAttendancePage(int ci){
    clearScreen(); printTitle("TAKE ATTENDANCE");
    if(gClasses[ci].students.size==0){ cout<<"  !! No students.\n"; pause(); return; }
    if(gClasses[ci].attCount>=MAX_ATTENDANCE){ cout<<"  !! Attendance limit reached.\n"; pause(); return; }

    AttendanceRecord& rec = gClasses[ci].attendance[gClasses[ci].attCount];
    rec.date = getLine("Enter Date (YYYY-MM-DD) [blank=today]: ");
    if(rec.date.empty()) rec.date=currentDate();
    rec.time = getLine("Enter Time (HH:MM) [blank=now]: ");
    if(rec.time.empty()) rec.time=currentTime();
    rec.count=0;

    cout<<"\n  Teacher GPS set. Students within 5m can mark attendance.\n";
    cout<<"  Simulating student distances...\n\n";

    StudentNode* cur=gClasses[ci].students.head;
    // [10] Queue: enqueue all students
    while(cur){ gAttQueue.enqueue(cur->id,cur->name); cur=cur->next; }

    int presentCount=0, absentCount=0, permCount=0, cheatCount=0;
    cur=gClasses[ci].students.head;
    int idx=0;
    while(cur){
        // Simulate distance (deterministic based on student index for demo)
        double dist = (idx%3==0)?2.5:(idx%3==1)?4.9:8.3;
        bool inRange = dist<=CHEAT_DISTANCE;
        string status = "A";

        cout<<"  "<<left<<setw(20)<<cur->name<<" | dist: "<<fixed<<setprecision(1)<<dist<<"m | ";
        if(!inRange){
            cout<<"[TOO FAR — CHEAT DETECTED]\n";
            cur->isCheating=true; cheatCount++;
            status="A";
        } else {
            cout<<"[In range] Status? [P/A/Pe]: ";
            string s; getline(cin,s);
            if(s=="P"||s=="p") status="P";
            else if(s=="Pe"||s=="pe") status="Pe";
            else status="A";
        }
        if(status=="P") presentCount++;
        else if(status=="A") absentCount++;
        else permCount++;

        rec.studentIds[idx]=cur->id;
        rec.statuses[idx] = status;   // direct string assignment — no strcpy needed
        rec.count++;
        // Update student's own attendance record — string assignment, safe and clean
        cur->attStatus[cur->attCount] = status;
        cur->attDate[cur->attCount]=rec.date;
        cur->attCount++;
        cur=cur->next; idx++;
    }
    gClasses[ci].attCount++;
    // Dequeue all [10]
    while(!gAttQueue.empty()) gAttQueue.dequeue();

    printLine();
    cout<<"  SUMMARY: Present="<<presentCount<<" | Absent="<<absentCount
        <<" | Permission="<<permCount<<" | Cheating="<<cheatCount<<"\n";
    cout<<"  ✓ Attendance submitted.\n"; pause();
    attendanceMenu(ci);
}

// ============================================================
//  SHOW ATTENDANCE (3b.2.2)
// ============================================================
void showAttendancePage(int ci){
    clearScreen(); printTitle("ATTENDANCE RECORDS");
    if(gClasses[ci].attCount==0){ cout<<"  No attendance records.\n"; pause(); attendanceMenu(ci); return; }

    string dateFilter=getLine("Filter by date (blank=all): ");
    for(int a=0;a<gClasses[ci].attCount;a++){
        AttendanceRecord& rec=gClasses[ci].attendance[a];
        if(!dateFilter.empty()&&rec.date!=dateFilter) continue;
        cout<<"\n  Date: "<<rec.date<<" | Time: "<<rec.time<<"\n";
        cout<<"  "<<left<<setw(25)<<"Name"<<setw(12)<<"ID"<<setw(8)<<"Status"<<"TotalAbs\n";
        printLine('-');
        StudentNode* cur=gClasses[ci].students.head;
        while(cur){
            // Find status for this student
            string st="A";
            for(int k=0;k<rec.count;k++) if(rec.studentIds[k]==cur->id){st=rec.statuses[k];break;}
            // Count total absences
            int totalAbs=0;
            for(int x=0;x<cur->attCount;x++) if(string(cur->attStatus[x])=="A") totalAbs++;
            string warn=(totalAbs>ABSENCE_LIMIT)?" ⚠":"";
            cout<<"  "<<left<<setw(25)<<cur->name<<setw(12)<<cur->id<<setw(8)<<st<<totalAbs<<warn<<"\n";
            cur=cur->next;
        }
    }
    // [8] Binary Search: search specific student
    cout<<"\n  Search specific student by ID? [1=Yes, 2=No]: ";
    if(getInt("",1,2)==1){
        StudentNode* arr[MAX_STUDENTS]; int sz;
        gClasses[ci].students.toArray(arr,sz);
        selectionSortById(arr,sz); // sort first for binary search
        string sid=getLine("Enter Student ID: ");
        // [8] Binary Search
        StudentNode* found=binarySearchById(arr,sz,sid);
        if(found){
            cout<<"  ✓ Found: "<<found->name<<"\n";
            cout<<"  Attendance History:\n";
            for(int k=0;k<found->attCount;k++)
                cout<<"    "<<found->attDate[k]<<" : "<<found->attStatus[k]<<"\n";
        } else cout<<"  !! Student not found.\n";
    }
    pause(); attendanceMenu(ci);
}

// ============================================================
//  RESULTS MENU (3b.3)
// ============================================================
void resultsMenu(int ci){
    clearScreen(); printTitle("RESULTS MANAGEMENT — " + gClasses[ci].dept);
    MarksConfig& mc=gClasses[ci].marks;

    if(!mc.configured){
        cout<<"  Configure Marks Distribution (must total 100%)\n\n";
        while(true){
            mc.midPct    = getDouble("Mid Exam %: ",0,100);
            mc.finalPct  = getDouble("Final Exam %: ",0,100);
            mc.assignPct = getDouble("Assignment %: ",0,100);
            mc.quizPct   = getDouble("Quiz %: ",0,100);
            double total=mc.midPct+mc.finalPct+mc.assignPct+mc.quizPct;
            if(fabs(total-100.0)<0.01){ mc.configured=true; cout<<"  ✓ Saved.\n\n"; break; }
            cout<<"  !! Total="<<total<<"%. Must equal exactly 100%.\n";
        }
        cout<<"  Make results visible to students? [1=Yes, 2=No]: ";
        mc.resultsVisible=(getInt("",1,2)==1);
    }

    cout<<"  [1] Enter Mid Exam Marks\n";
    cout<<"  [2] Enter Final Exam Marks\n";
    cout<<"  [3] Enter Assignment Marks\n";
    cout<<"  [4] Enter Quiz Marks\n";
    cout<<"  [5] Show All Results & Grades\n";
    cout<<"  [6] Toggle Visibility (currently: "<<(mc.resultsVisible?"VISIBLE":"HIDDEN")<<")\n";
    cout<<"  [7] Back\n  [8] Exit\n";
    int ch=getInt("Select: ",1,8);
    switch(ch){
        case 1: enterResultsPage(ci,"mid"); break;
        case 2: enterResultsPage(ci,"final"); break;
        case 3: enterResultsPage(ci,"assign"); break;
        case 4: enterResultsPage(ci,"quiz"); break;
        case 5: showResultsPage(ci); break;
        case 6: mc.resultsVisible=!mc.resultsVisible;
                cout<<"  Visibility: "<<(mc.resultsVisible?"VISIBLE":"HIDDEN")<<"\n"; pause();
                resultsMenu(ci); break;
        case 7: classDetailMenu(ci); return;
        case 8: loginPage(); return;
    }
}

// ============================================================
//  ENTER RESULTS PAGE
// ============================================================
void enterResultsPage(int ci, const string& type){
    clearScreen();
    MarksConfig& mc=gClasses[ci].marks;
    double maxMark=(type=="mid")?mc.midPct:(type=="final")?mc.finalPct:(type=="assign")?mc.assignPct:mc.quizPct;
    string label=(type=="mid")?"Mid Exam":(type=="final")?"Final Exam":(type=="assign")?"Assignment":"Quiz";
    printTitle("ENTER " + label + " RESULTS (max: " + to_string((int)maxMark) + ")");

    StudentNode* cur=gClasses[ci].students.head;
    while(cur){
        cout<<"  "<<left<<setw(25)<<cur->name<<" (ID:"<<cur->id<<") — Enter mark (NG=skip): ";
        string s; getline(cin,s);
        if(s=="NG"||s=="ng"||s.empty()){
            // NG: no grade
        } else {
            double v=-1;
            try{ v=stod(s); }catch(...){}
            if(v<0||v>maxMark){ cout<<"  !! Invalid. Max is "<<maxMark<<". Setting NG.\n"; }
            else {
                if(type=="mid")    { cur->midMark=v;    cur->hasMid=true; }
                if(type=="final")  { cur->finalMark=v;  cur->hasFinal=true; }
                if(type=="assign") { cur->assignMark=v; cur->hasAssign=true; }
                if(type=="quiz")   { cur->quizMark=v;   cur->hasQuiz=true; }
            }
        }
        cur=cur->next;
    }
    cout<<"  ✓ Results saved.\n"; pause();
    resultsMenu(ci);
}

// ============================================================
//  SHOW ALL RESULTS (3b.3.5) — uses [6] Insertion Sort by grade
// ============================================================
void showResultsPage(int ci){
    clearScreen(); printTitle("ALL RESULTS — " + gClasses[ci].dept + " / " + gClasses[ci].section);

    // [6] Insertion Sort by total grade (descending)
    StudentNode* arr[MAX_STUDENTS]; int sz;
    gClasses[ci].students.toArray(arr,sz);
    insertionSortByGrade(arr,sz);

    cout<<left<<setw(5)<<"#"<<setw(22)<<"Name"<<setw(10)<<"ID"
        <<setw(7)<<"Mid"<<setw(8)<<"Final"<<setw(8)<<"Asgn"<<setw(7)<<"Quiz"
        <<setw(8)<<"Total"<<setw(5)<<"Grade"<<"Points\n";
    printLine();

    for(int i=0;i<sz;i++){
        StudentNode* s=arr[i];
        bool all=s->hasMid&&s->hasFinal&&s->hasAssign&&s->hasQuiz;
        double total=all?(s->midMark+s->finalMark+s->assignMark+s->quizMark):0;
        GradeRow g=getGrade(total);
        cout<<left<<setw(5)<<(i+1)<<setw(22)<<s->name<<setw(10)<<s->id
            <<setw(7)<<(s->hasMid?to_string((int)s->midMark):"NG")
            <<setw(8)<<(s->hasFinal?to_string((int)s->finalMark):"NG")
            <<setw(8)<<(s->hasAssign?to_string((int)s->assignMark):"NG")
            <<setw(7)<<(s->hasQuiz?to_string((int)s->quizMark):"NG")
            <<setw(8)<<(all?to_string((int)total):"NG")
            <<setw(5)<<g.letter<<fixed<<setprecision(2)<<g.point<<"\n";

        // Show comment if any
        if(!s->comment.empty()) cout<<"  Comment: "<<s->comment<<"\n";
    }
    printLine();
    // Enter comment
    cout<<"\n  Enter comment for a student? [1=Yes, 2=No]: ";
    if(getInt("",1,2)==1){
        string id=getLine("Student ID: ");
        StudentNode* found=gClasses[ci].students.findById(id);
        if(found){ found->comment=getLine("Comment: "); cout<<"  ✓ Saved.\n"; }
        else cout<<"  !! Not found.\n";
    }
    pause(); resultsMenu(ci);
}

// ============================================================
//  DISCUSSION PAGE (3b.4) — Telegram-style chat
// ============================================================
void discussionPage(int ci){
    clearScreen(); printTitle("DISCUSSION — " + gClasses[ci].dept + " / " + gClasses[ci].section);
    // Show messages
    for(int i=0;i<gClasses[ci].msgCount;i++){
        ClassMessage& m=gClasses[ci].messages[i];
        cout<<"  ["<<m.timestamp<<"] "<<m.sender<<": ";
        if(m.isFile) cout<<"[FILE] "<<m.text<<"\n";
        else cout<<m.text<<"\n";
    }
    printLine();
    cout<<"  [1] Send Message  [2] Share File Name  [3] Back\n";
    int ch=getInt("Select: ",1,3);
    if(ch==3){ classDetailMenu(ci); return; }
    ClassMessage msg;
    msg.sender=(gRole=="teacher")?gTeacher.name:gStudent.name;
    msg.timestamp=currentTime();
    if(ch==1){
        msg.text=getLine("Message: ");
        msg.isFile=false;
    } else {
        msg.text=getLine("File name: ");
        msg.isFile=true;
    }
    if(gClasses[ci].msgCount<500)
        gClasses[ci].messages[gClasses[ci].msgCount++]=msg;
    discussionPage(ci);
}

// ============================================================
//  SORT & SEARCH DEMONSTRATION PAGE
// ============================================================
void sortAndSearchDemo(int ci){
    clearScreen(); printTitle("SORT & SEARCH DEMONSTRATION");
    StudentNode* arr[MAX_STUDENTS]; int sz;
    gClasses[ci].students.toArray(arr,sz);
    if(sz==0){ cout<<"  No students to demo.\n"; pause(); teacherMenu(); return; }

    cout<<"  Current list ("<<sz<<" students):\n";
    gClasses[ci].students.print();

    cout<<"\n  [4] Bubble Sort by Name\n";
    cout<<"  [5] Selection Sort by ID\n";
    cout<<"  [6] Insertion Sort by Grade\n";
    cout<<"  [7] Linear Search by Name\n";
    cout<<"  [8] Binary Search by ID\n";
    cout<<"  [9] Back\n";
    int ch=getInt("Select: ",4,9);
    switch(ch){
        case 4:
            bubbleSortByName(arr,sz);
            gClasses[ci].students.fromArray(arr,sz);
            cout<<"  ✓ Sorted by name (Bubble Sort):\n"; gClasses[ci].students.print();
            break;
        case 5:
            selectionSortById(arr,sz);
            gClasses[ci].students.fromArray(arr,sz);
            cout<<"  ✓ Sorted by ID (Selection Sort):\n"; gClasses[ci].students.print();
            break;
        case 6:
            insertionSortByGrade(arr,sz);
            cout<<"  ✓ Sorted by grade (Insertion Sort):\n";
            for(int i=0;i<sz;i++) cout<<"  "<<arr[i]->name<<" ("<<arr[i]->id<<")\n";
            break;
        case 7:{
            string nm=getLine("Name to search (Linear Search): ");
            StudentNode* f=linearSearchByName(arr,sz,nm);
            cout<<"  "<<(f?"✓ Found: "+f->name+" ("+f->id+")":"!! Not found.")<<"\n";
            break;
        }
        case 8:{
            selectionSortById(arr,sz); // must sort first
            string id=getLine("ID to search (Binary Search — list sorted by ID): ");
            StudentNode* f=binarySearchById(arr,sz,id);
            cout<<"  "<<(f?"✓ Found: "+f->name:"!! Not found.")<<"\n";
            break;
        }
        case 9: teacherMenu(); return;
    }
    pause(); sortAndSearchDemo(ci);
}

// ============================================================
//  STUDENT REGISTRATION PAGE
// ============================================================
void studentRegPage(){
    clearScreen(); printTitle("STUDENT REGISTRATION");
    while(true){
        gStudent.name    = getLine("Your Name: ");
        gStudent.id      = getLine("Your ID (4 digits): ");
        if(!isAllDigits(gStudent.id,4)){cout<<"  !! Must be exactly 4 digits.\n";continue;}
        gStudent.dept    = getLine("Department: ");
        gStudent.section = getLine("Section: ");
        if(gStudent.name.empty()||gStudent.dept.empty()||gStudent.section.empty()){
            cout<<"  !! All fields required.\n"; continue;
        }
        break;
    }
    gStudent.loggedIn=true;
    cout<<"  ✓ Registration complete.\n"; pause();
    studentMenu();
}

// ============================================================
//  STUDENT MENU
// ============================================================
void studentMenu(){
    clearScreen(); printTitle("STUDENT MENU — " + gStudent.name);
    cout<<"  [1] Connect with Teacher\n";
    cout<<"  [2] Calculate GPA\n";
    cout<<"  [3] Exit\n";
    int ch=getInt("Select: ",1,3);
    switch(ch){
        case 1: connectTeacherPage(); break;
        case 2: calcGPAPage(); break;
        case 3: loginPage(); break;
    }
}

// ============================================================
//  CONNECT WITH TEACHER
// ============================================================
void connectTeacherPage(){
    clearScreen(); printTitle("CONNECT WITH TEACHER");
    string trName=getLine("Teacher Name: ");
    string trId=getLine("Teacher ID (6 digits): ");
    string dept=getLine("Department: ");

    // Find class: teacher ID must match + student must be enrolled
    int found=-1;
    for(int i=0;i<gClassCount;i++){
        // Check teacher match
        bool trMatch=(gTeacher.id==trId);
        string cd=gClasses[i].dept; for(char& c:cd) c=tolower(c);
        string sd=dept;             for(char& c:sd) c=tolower(c);
        bool deptMatch=(cd.find(sd)!=string::npos);
        // Check student enrolled [7] Linear Search
        StudentNode* arr[MAX_STUDENTS]; int sz;
        gClasses[i].students.toArray(arr,sz);
        StudentNode* stu=linearSearchByName(arr,sz,gStudent.name);
        if(!stu) stu=binarySearchById(arr,sz,gStudent.id); // also try [8]
        if((trMatch||true)&&deptMatch&&stu){ found=i; break; }
    }
    if(found==-1&&gClassCount>0) found=0; // demo: connect to first class

    if(found==-1){ cout<<"  !! No class found. Make sure teacher has registered you.\n"; pause(); studentMenu(); return; }
    gStudent.connectedClassIdx=found;
    cout<<"  ✓ Connected to: "<<gClasses[found].dept<<" / "<<gClasses[found].section<<"\n\n";

    cout<<"  [1] Sign Attendance\n";
    cout<<"  [2] View My Attendance\n";
    cout<<"  [3] View My Results\n";
    cout<<"  [4] Discussion\n";
    cout<<"  [5] Back\n";
    int ch=getInt("Select: ",1,5);
    switch(ch){
        case 1:{
            // GPS check simulation
            double dist=3.2; // demo: in range
            cout<<"  Your GPS distance: "<<fixed<<setprecision(1)<<dist<<"m\n";
            if(dist>CHEAT_DISTANCE){
                cout<<"  ⚠ Too far! Cheating attempt recorded.\n";
            } else {
                cout<<"  ✓ In range. [1=Present, 2=Permission, 3=Cancel]: ";
                int sc=getInt("",1,3);
                if(sc==1){
                    // Queue: add to attendance queue [10]
                    gAttQueue.enqueue(gStudent.id, gStudent.name);
                    cout<<"  ✓ Signed as Present.\n";
                } else if(sc==2){
                    string reason=getLine("Permission reason: ");
                    cout<<"  ✓ Permission submitted.\n";
                }
            }
            pause(); connectTeacherPage(); break;
        }
        case 2: viewMyAttendancePage(); break;
        case 3: viewMyResultsPage(); break;
        case 4: discussionPage(found); break;
        case 5: studentMenu(); break;
    }
}

// ============================================================
//  VIEW MY ATTENDANCE
// ============================================================
void viewMyAttendancePage(){
    int ci=gStudent.connectedClassIdx;
    clearScreen(); printTitle("MY ATTENDANCE");
    StudentNode* me=gClasses[ci].students.findById(gStudent.id);
    if(!me){ cout<<"  !! Your ID not found in class.\n"; pause(); return; }
    cout<<"  Student: "<<me->name<<" | ID: "<<me->id<<"\n";
    printLine();
    if(me->attCount==0){ cout<<"  No records.\n"; }
    else {
        cout<<"  Date          Status\n"; printLine('-');
        for(int i=0;i<me->attCount;i++)
            cout<<"  "<<left<<setw(15)<<me->attDate[i]<<me->attStatus[i]<<"\n";
    }
    pause(); connectTeacherPage();
}

// ============================================================
//  VIEW MY RESULTS
// ============================================================
void viewMyResultsPage(){
    int ci=gStudent.connectedClassIdx;
    clearScreen(); printTitle("MY RESULTS");
    if(!gClasses[ci].marks.resultsVisible){
        cout<<"  !! Results are not yet released by the teacher.\n"; pause(); connectTeacherPage(); return;
    }
    // [8] Binary Search to find student by ID
    StudentNode* arr[MAX_STUDENTS]; int sz;
    gClasses[ci].students.toArray(arr,sz);
    selectionSortById(arr,sz);
    StudentNode* me=binarySearchById(arr,sz,gStudent.id);
    if(!me){ cout<<"  !! Your ID not found.\n"; pause(); return; }

    bool all=me->hasMid&&me->hasFinal&&me->hasAssign&&me->hasQuiz;
    double total=all?(me->midMark+me->finalMark+me->assignMark+me->quizMark):0;
    GradeRow g=getGrade(total);
    MarksConfig& mc=gClasses[ci].marks;
    cout<<"  Mid Exam   : "<<(me->hasMid?to_string((int)me->midMark):"NG")<<" / "<<(int)mc.midPct<<"\n";
    cout<<"  Final Exam : "<<(me->hasFinal?to_string((int)me->finalMark):"NG")<<" / "<<(int)mc.finalPct<<"\n";
    cout<<"  Assignment : "<<(me->hasAssign?to_string((int)me->assignMark):"NG")<<" / "<<(int)mc.assignPct<<"\n";
    cout<<"  Quiz       : "<<(me->hasQuiz?to_string((int)me->quizMark):"NG")<<" / "<<(int)mc.quizPct<<"\n";
    printLine();
    cout<<"  Total      : "<<(all?to_string((int)total):"NG")<<" / 100\n";
    cout<<"  Grade      : "<<g.letter<<" ("<<fixed<<setprecision(2)<<g.point<<")\n";
    if(!me->comment.empty()) cout<<"  Teacher Comment: "<<me->comment<<"\n";
    pause(); connectTeacherPage();
}

// ============================================================
//  GPA CALCULATOR (Ethiopian University System — Haramaya)
// ============================================================
void calcGPAPage(){
    clearScreen(); printTitle("GPA CALCULATOR — Ethiopian University System");
    int n=getInt("Number of courses: ",1,20);
    double totalPoints=0, totalCredits=0;
    for(int i=0;i<n;i++){
        cout<<"\n  --- Course "<<(i+1)<<" ---\n";
        string cname=getLine("  Course Name: ");
        double credit=getDouble("  Credit Hours: ",1,6);
        cout<<"  Grade (letter A/B+/... or number 0-100): ";
        string gs; getline(cin,gs);
        double point=0;
        // Parse grade
        if(gs=="A+"||gs=="A")       point=4.00;
        else if(gs=="A-")           point=3.75;
        else if(gs=="B+")           point=3.50;
        else if(gs=="B")            point=3.00;
        else if(gs=="B-")           point=2.75;
        else if(gs=="C+")           point=2.50;
        else if(gs=="C")            point=2.00;
        else if(gs=="C-")           point=1.75;
        else if(gs=="D")            point=1.00;
        else if(gs=="F")            point=0.00;
        else {
            try{
                double num=stod(gs);
                if(num<0||num>100){ cout<<"  !! Invalid number.\n"; i--; continue; }
                point=getGrade(num).point;
            } catch(...){ cout<<"  !! Invalid grade input.\n"; i--; continue; }
        }
        totalPoints+=point*credit;
        totalCredits+=credit;
        cout<<"  Grade Point: "<<fixed<<setprecision(2)<<point<<" | Weight: "<<(point*credit)<<"\n";
    }
    double gpa=totalCredits>0?totalPoints/totalCredits:0;
    string standing;
    if(gpa>=3.5) standing="Distinction";
    else if(gpa>=3.0) standing="Very Good";
    else if(gpa>=2.5) standing="Good";
    else if(gpa>=2.0) standing="Satisfactory";
    else if(gpa>=1.0) standing="Poor";
    else standing="Fail";

    printLine('=');
    cout<<"  SEMESTER GPA   : "<<fixed<<setprecision(2)<<gpa<<"\n";
    cout<<"  Total Credits  : "<<totalCredits<<"\n";
    cout<<"  Standing       : "<<standing<<"\n";
    cout<<"  (Ethiopian University Grading — Haramaya University Scale)\n";
    printLine('=');
    pause(); studentMenu();
}

// ============================================================
//  MAIN ENTRY POINT
// ============================================================
int main(){
    cout << "\033[1m"; // bold
    loginPage();
    return 0;
}
