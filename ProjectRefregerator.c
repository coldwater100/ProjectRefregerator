
#include <stdio.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>


#define ARROW_DOWN 80
#define ARROW_UP 72
#define ARROW_LEFT 75
#define ARROW_RIGHT 77
#define COUNT_OF_MAIN_MENU 6

#define NOT_SORTED 101
#define ASC_SORTED 102
#define DESC_SORTED 103

#define FOOD_NUMBER_MAX 5 //레시피 내 음식 갯수 최대값
#define ANSI_COLOR_RED     "\x1b[31m" // 색상을 붉은색으로 변경합니다.
#define ANSI_COLOR_RESET   "\x1b[0m" // 색상을 기본값으로 변경합니다.

// 보관 물품 항목에 대한 구조체
typedef struct {
    char name[20];
    int count;
    time_t start_date;
    time_t expire_date;
} _ITEM;

// 레시피 물품 항목에 대한 구조체
typedef struct {
    char name[20];
    int count;
} _RECIPE_ITEM;

// 커서 포지션에 대한 구조체
typedef struct {
    int x;
    int y;
} _CURSOR_POS;

typedef struct  { // 레시피 정보입니다.
    char name[30]; // 레시피 이름입니다.
    _RECIPE_ITEM food[FOOD_NUMBER_MAX]; // 레시피 내 음식 정보입니다.
} _RECIPE;

// main menu의 enum 상수
enum MENU_ITEM {
    CURRENT_STATUS, REMOVE_FOOD, NEW_FOOD, MODIFY_ITEM, RECIPE_MANAGE, EXIT
};
// 현재  select 된 main menu
enum MENU_ITEM selectedMenu = CURRENT_STATUS;

// 냉장고 상태보기 내에서 사용되는 enum 상수
enum STATUS_BUTTON_MENU {
    RECIPE, REF_TEMP_UP, REF_TEMP_DOWN, FRE_TEMP_UP, FRE_TEMP_DOWN,
    SORT_NAME, SORT_START_DATE, SORT_EXPIRE_DATE, PREVIOUS
};
// 냉장고 상태보기 내에서 사용되는 정렬 상태 표시 변수
int name_sorted = NOT_SORTED;
int start_date_sorted = NOT_SORTED;
int expire_date_sorted = NOT_SORTED;

// 파일에 쓰기 및 읽기용  파일 이름
const char* strFileName = "refregirator.rfr";
const char* strRecipeFileName = "recipe.rfr";


// console의 cursor 관련 함수
int getCursorPosition(_CURSOR_POS* cursor_pos);
int setCursorPosition(_CURSOR_POS* cursor_pos);
int clearConsole();

// 기본 메뉴 표시 화면
void printMenu();

// option에 따른  함수의 분기 지정
void doAction(int option);

// 현재냉장고 상태  와 _ITEM*   목록 보여주기
void doListItems();
void printRefregiratorStatus(enum STATUS_BUTTON_MENU currentSelection);

// _ITEM 항목 삭제 
int doDeleteItem(); // remove 기초 작업
int removeFromReservedItem(int itemNo); // 실제 remove 기능

// 새로운 아이템 추가
int doAddNewItem(); // add 기초 작업
int addReservedItem(_ITEM* item); // 실제 add 기능

// 저장된 아이템을 수정하는 기능
//int doModifyItem(); // 수정 기초 작업
//int modifyReservedItem(int select); // 실제 수정 기능

// 저장된 file에서 불러 오기 및 쓰기
int doReadFromFile();
int doSaveToFile();



// RECIPE 항목 삭제 
//int doDeleteRecipe(); // remove 기초 작업
//int removeFromReservedRecipe(int itemNo); // 실제 remove 기능

void doPrintRecepi();//레시피 출력


// 새로운 레시피 추가
int doAddNewRecipe(); // add 기초 작업
int addReservedRecipe(_RECIPE* item); // 실제 add 기능

// 저장된 레시피를 수정하는 기능
//int doModifyRecipe(); // 수정 기초 작업
//int modifyReservedRecipe(int select); // 실제 수정 기능

// 저장된 file에서 레시피 불러 오기 및 쓰기
int doRecipeReadFromFile();
int doRecipeSaveToFile();


// qsort 함수(stdlib) callback에 들어갈 함수들
int compareItemsByNameASC(const _ITEM* a, const _ITEM* b);
int compareItemsByNameDESC(const _ITEM* a, const _ITEM* b);
int compareItemsByStartDateASC(const _ITEM* a, const _ITEM* b);
int compareItemsByStartDateDESC(const _ITEM* a, const _ITEM* b);
int compareItemsByExpireDateASC(const _ITEM* a, const _ITEM* b);
int compareItemsByExpireDateDESC(const _ITEM* a, const _ITEM* b);

// 위의 함수를 보조하기 위한  함수
int printReservedItem();  // 현재 지정된 항목을 list하기 위한 함수
char* getFormatedStringByTime_t(time_t* ttCurrent);  // time_t에서  날짜 스트링으로 변환 실행측에서 memory해제해야 함
time_t inputDateFromConsole(const char* message);  // time_t 구조를 입력 받기 위한 함수
void setTextHighlight(); // console의 글자 색을  강조색으로
void setTextNormal(); // consoel의 글자 색을 흰색으로

void draw_line1(int line_length); // 괄호 내 칸 만큼 줄을 긋고 한 줄을 띄웁니다.
void draw_line2(int line_length); // 괄호 내 칸 만큼 이중으로 된 줄을 긋고 한 줄을 띄웁니다.

int doRecipePage();//레시피 화면 구현 함수
void displayRecipeMenu(int selected); // 메인메뉴입니다.

void recipe_init(_RECIPE_ITEM* newRecipeItem); //레시피 전체의 음식 이름은 "", 개수는 0으로 전부 초기화됩니다.



// 기본 화면에 표시할 메뉴 문자열
const char* MENU[] = {
    "1 >> 현재 상황 및 음식 출력\n",
    "2 >> 냉장고 음식 삭제\n",
    "3 >> 냉장고 음식 추가\n",
    "4 >> 냉장고 음식 수정\n",
    "5 >> 레시피 관리\n",
    "6 >> 종료\n"
};


// 현재 냉장고 보관 물품 목록
_ITEM* reservedItem=NULL;

//레시피 목록
_RECIPE* recipe = NULL;

// 현재 냉장고 보관 품목 수
//reservedItem과 항상 동기를 정확히 시켜주어야 함
int countOfItems = 0;

// 현재 레시피 수
//recipe과 항상 동기를 정확히 시켜주어야 함
int countOfRecipe = 0;

// 냉장고 온도
int refregiratorTemperature = 5;
int freezerTemperature = -5;

int main()
{
    if (doReadFromFile() == 0) { printf("Error !!! doReadFromFile()"); exit(1); }
    if (doRecipeReadFromFile() == 0) { printf("Error !!! doReadFromFile()"); exit(1); }

    printMenu();

    char arrow; // arrow를 누르면 224, ARROW_DOWN, ARROW_UP 값이 순차적으로 들어 옴

    // arrow key이면 selectedMenu를 바꾸고, 메뉴 해당 숫자나, enter key이면  현재의 selectedMenu를 실행 시킴
    while (1) {
        switch (_getch()) {
        case 224: //특수 키면 ArrowUp인지 ArrowDown인지 확인
            arrow = _getch();
            if (arrow == ARROW_DOWN || arrow == ARROW_RIGHT) {
                if (selectedMenu < COUNT_OF_MAIN_MENU - 1) selectedMenu = (enum MENU_ITEM)(selectedMenu + 1);
            }
            else if (arrow == ARROW_UP || arrow == ARROW_LEFT) {
                if (selectedMenu > 0) selectedMenu = (enum MENU_ITEM)(selectedMenu - 1);
            }
            break;
        case '1'://숫자키 입력으로 메뉴 바로가기
            doAction(0);
            break;
        case '2':
            doAction(1);
            break;
        case '3':
            doAction(2);
            break;
        case '4':
            doAction(3);
            break;
        case '5':
            doAction(4);
            break;
        case '6':
            doAction(7);
            break;
        case '\r':  // Enter key가 눌려지면 실제 행동을 한다
            doAction(selectedMenu);
        default:
            break;
        }
        printMenu();
    }
}

// main menu를 display,  현재 selecetedMenu의 값에 따라 text를 강조 색으로 바꾼다.
void printMenu() {
    clearConsole();
    printf("*********************************************************\n");
    printf("방향키를 이용해서 커서를 선택할 메뉴로 이동한 뒤 엔터를 누르거나\n실행하고자 하는 메뉴의 숫자를 입력하세요\n\n");
    printf("*********************************************************\n");
    int countOfMenu = sizeof(MENU) / sizeof(MENU[0]);
    for (int i = 0; i < countOfMenu; i++) {
        if (selectedMenu == i) setTextHighlight();
        printf("%s", MENU[i]);
        setTextNormal();
    }
    printf("***********************************\n");
}

// 메인메뉴에서 엔터키가 눌렸을때 호출 되는 함수
// option 에 따라 해당 function을 실행 시킨다.
void doAction(int option) {

    switch (option) {
    case CURRENT_STATUS:
        doListItems();
        break;
    case REMOVE_FOOD:
        if (doDeleteItem() == 0) { printf("Error !!! doDeleteItem()"); exit(1); };
        break;
    case NEW_FOOD:
        if (doAddNewItem() == 0) { printf("Error !!! doAddNewItem()"); exit(1); };
        break;
    case MODIFY_ITEM:
        if (doModifyItem() == 0) { printf("Error !!! doModifyItem()"); exit(1); }
        break;
    case RECIPE_MANAGE:
        if (doRecipePage() == 0) { printf("Error !!! doModifyItem()"); exit(1); }
        break;
    case EXIT:
        exit(0);
    default:break;
    }
}

// 현재 냉장고의 상태와 전체 보관 항목을 표시해 주는 역할
void doListItems() {

    // 현재 선택된 메뉴 설정
    enum STATUS_BUTTON_MENU currentSelection = RECIPE;

    //  화살표 키 입력을 받을 변수
    int arrow;

    // 현상태를 인쇄
    printRefregiratorStatus(currentSelection);

    int completed = 0;
    //  4개의 화살표 버튼이 눌림에 따라  currentSelection 변수의 값을 변화 시킴,  Enter Key가 눌리면  currentSelection을 실행시킴
    while (!completed) {
        switch (_getch()) {
        case 224: //특수 키면 ArrowUp인지 ArrowDown인지 확인
            arrow = _getch();
            if (arrow == ARROW_DOWN || arrow == ARROW_RIGHT) {
                if (currentSelection < 8) currentSelection = (enum STATUS_BUTTON_MENU)(currentSelection + 1);
            }
            else if (arrow == ARROW_UP || arrow == ARROW_LEFT) {
                if (currentSelection > 0) currentSelection = (enum STATUS_BUTTON_MENU)(currentSelection - 1);
            }
            break;

        case '\r':  // Enter key가 눌려지면 실제 행동을 한다
            switch (currentSelection) {
            case RECIPE:
                doRecipePage();
                break;
            case REF_TEMP_UP:
                refregiratorTemperature++;
                break;
            case REF_TEMP_DOWN:
                refregiratorTemperature--;
                break;
            case FRE_TEMP_UP:
                freezerTemperature++;
                break;
            case FRE_TEMP_DOWN:
                freezerTemperature--;
                break;
            case SORT_NAME:
                switch (name_sorted) {
                case NOT_SORTED:
                    qsort(reservedItem, countOfItems, sizeof(_ITEM), compareItemsByNameASC);
                    name_sorted = ASC_SORTED;  start_date_sorted = NOT_SORTED;  expire_date_sorted = NOT_SORTED;   break;
                case ASC_SORTED:
                    qsort(reservedItem, countOfItems, sizeof(_ITEM), compareItemsByNameDESC);
                    name_sorted = DESC_SORTED;  start_date_sorted = NOT_SORTED;  expire_date_sorted = NOT_SORTED;   break;
                case DESC_SORTED:
                    qsort(reservedItem, countOfItems, sizeof(_ITEM), compareItemsByNameASC);
                    name_sorted = ASC_SORTED;  start_date_sorted = NOT_SORTED;  expire_date_sorted = NOT_SORTED;   break;
                }
                break;
            case SORT_START_DATE:
                switch (start_date_sorted) {
                case NOT_SORTED:
                    qsort(reservedItem, countOfItems, sizeof(_ITEM), compareItemsByStartDateASC);
                    name_sorted = NOT_SORTED;  start_date_sorted = ASC_SORTED;  expire_date_sorted = NOT_SORTED;   break;
                case ASC_SORTED:
                    qsort(reservedItem, countOfItems, sizeof(_ITEM), compareItemsByStartDateDESC);
                    name_sorted = NOT_SORTED;  start_date_sorted = DESC_SORTED;  expire_date_sorted = NOT_SORTED;   break;
                case DESC_SORTED:
                    qsort(reservedItem, countOfItems, sizeof(_ITEM), compareItemsByStartDateASC);
                    name_sorted = NOT_SORTED;  start_date_sorted = ASC_SORTED;  expire_date_sorted = NOT_SORTED;   break;
                }
                break;
            case SORT_EXPIRE_DATE:
                switch (expire_date_sorted) {
                case NOT_SORTED:
                    qsort(reservedItem, countOfItems, sizeof(_ITEM), compareItemsByExpireDateASC);
                    name_sorted = NOT_SORTED;  start_date_sorted = NOT_SORTED;  expire_date_sorted = ASC_SORTED;   break;
                case ASC_SORTED:
                    qsort(reservedItem, countOfItems, sizeof(_ITEM), compareItemsByExpireDateDESC);
                    name_sorted = NOT_SORTED;  start_date_sorted = NOT_SORTED;  expire_date_sorted = DESC_SORTED;   break;
                case DESC_SORTED:
                    qsort(reservedItem, countOfItems, sizeof(_ITEM), compareItemsByExpireDateASC);
                    name_sorted = NOT_SORTED;  start_date_sorted = NOT_SORTED;  expire_date_sorted = ASC_SORTED;   break;
                }
                break;
            case PREVIOUS:
                completed = 1;
                break;
            default:
                break;
            }
        }
        printRefregiratorStatus(currentSelection);
    }

}

// 냉장고의 상태및 냉장고의 품목을 표시.
// currentSelection에 해당하는 가상 버튼의 Text 의 색깔을 TextHighlight로 바꾸어줌
void printRefregiratorStatus(enum STATUS_BUTTON_MENU currentSelection) {
    clearConsole();

    // 현재 시간
    time_t currentTime_t;
    time(&currentTime_t);

    printf("-------------------------------------------------------------------");
    printf("\n냉장고 관리 시스템\n");
    // 현재 시간을 구조체로 변환
    struct tm* localTime = localtime(&currentTime_t);

    printf("%d-%d-%d", 1900+localTime->tm_year, 1+localTime->tm_mon, localTime->tm_mday);
    switch (localTime->tm_wday) {
        case(0):
            printf("(일)\n");
            break;
        case(1):
            printf("(월)\n");
            break;
        case(2):
            printf("(화)\n");
            break;
        case(3):
            printf("(수)\n");
            break;
        case(4):
            printf("(목)\n");
            break;
        case(5):
            printf("(금)\n");
            break;
        case(6):
            printf("(토)\n");
            break;
    }
        

    printf("\n-------------------------------------------------------------------");
    printf("\n유통기한 임박");
    for (int i = 0; i < countOfItems; i++) {
        if (reservedItem[i].expire_date < currentTime_t) printf("\n%20s 가 유통기한이 경과하였습니다", reservedItem[i].name);
        else if (difftime(reservedItem[i].expire_date, currentTime_t) < 60 * 60 * 24) printf("\n%20s가 유통기한이 하루 이하입니다", reservedItem[i].name);
    }

    printf("\n-------------------------------------------------------------------");
    printf("\n지금 만들 수 있는 메뉴\n");
    if (currentSelection == RECIPE) setTextHighlight();
    printf("[레시피 관리]");
    setTextNormal();

    printf("\n-------------------------------------------------------------------");
    printf("\n냉장실(%3d도)", refregiratorTemperature);

    if (currentSelection == REF_TEMP_UP) setTextHighlight();
    printf(" [온도올리기] ");
    setTextNormal();

    if (currentSelection == REF_TEMP_DOWN) setTextHighlight();
    printf(" [온도내리기] ");
    setTextNormal();

    if (refregiratorTemperature < 0)  printf("  냉장실 온도가 너무 낮습니다");
    if (refregiratorTemperature > 4) printf("  냉장실 온도가 너무 높습니다");

    printf("\n냉동실(%3d도)", freezerTemperature);

    if (currentSelection == FRE_TEMP_UP) setTextHighlight();
    printf(" [온도올리기] ");
    setTextNormal();

    if (currentSelection == FRE_TEMP_DOWN) setTextHighlight();
    printf(" [온도내리기] ");
    setTextNormal();

    if (refregiratorTemperature < -20) printf("  냉장실 온도가 너무 낮습니다");
    if (freezerTemperature > -10) printf("  냉동실 온도가 너무 높습니다");

    // printReservedItem();  button을 위해 직접 인쇄
    printf("\n------------------------------------------------------------------------------------------------\n");
    printf("번호   ");

    // Sort 상태에 따라 세모, 역세모, 네모를 표시해줌
    if (currentSelection == SORT_NAME) setTextHighlight();
    switch (name_sorted) {
    case NOT_SORTED:  printf("\u25A0");  break;
    case ASC_SORTED: printf("\u25B2"); break;
    case DESC_SORTED:  printf("\u25BC"); break;
    }
    setTextNormal();
    printf(" 품목 이름             갯수        ");

    // Sort 상태에 따라 세모, 역세모, 네모를 표시해줌
    if (currentSelection == SORT_START_DATE) setTextHighlight();
    switch (start_date_sorted) {
    case NOT_SORTED:  printf("\u25A0");  break;
    case ASC_SORTED: printf("\u25B2"); break;
    case DESC_SORTED:  printf("\u25BC"); break;
    }
    setTextNormal();
    printf("입고날자            ");

    // Sort 상태에 따라 세모, 역세모, 네모를 표시해줌
    if (currentSelection == SORT_EXPIRE_DATE) setTextHighlight();
    switch (expire_date_sorted) {
    case NOT_SORTED:  printf("\u25A0");  break;
    case ASC_SORTED: printf("\u25B2"); break;
    case DESC_SORTED:  printf("\u25BC"); break;
    }
    setTextNormal();
    printf("유통기한");

    // 실제 보관 물품 목록 표시
    printf("\n------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < countOfItems; i++) {
        char* start_date = getFormatedStringByTime_t(&reservedItem[i].start_date);
        char* expire_date = getFormatedStringByTime_t(&reservedItem[i].expire_date);
        printf("%3d      %-20s  %3d개     %-20s  %-20s\n", i+1, reservedItem[i].name, reservedItem[i].count, start_date, expire_date);
        free(start_date);
        free(expire_date);
    }
    printf("------------------------------------------------------------------------------------------------\n");

    // 이전화면으로 버튼 표시
    if (currentSelection == PREVIOUS) setTextHighlight();
    printf("\n[이전화면으로]\n");
    setTextNormal();

    printf("\n상 하 좌 우  화살표 키로 선택할 메뉴로 이동한 뒤 엔터키를 누르세요\n");
    printf("\n네모(또는 삼각형,역삼각형) 버튼 선택 후 엔터키를 누르면 정렬 기준(ex 유통기한 오름차순 기준 정렬)을 바꿀 수 있습니다\n");
}

// 새로운 항목을 추가 해 줌 - 실제 기능은 addReservedItem() 함수가 시행
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int doAddNewItem() {
    _ITEM newItem;

    clearConsole();
    printf("보관할 새로운 정보를 입력합니다 \n\n");
    // char name[20] '\0' 를 위해 19자만 입력 가능
    printf("품목 이름 : (19자이내) >> ");
    scanf("%s", newItem.name);
    printf("품목 개수(단위는 빼고 입력하세요) >> ");
    scanf("%d", &newItem.count);

    // start_time에 현재 날자 배정
    time(&newItem.start_date);
    char* startDate = getFormatedStringByTime_t(&newItem.start_date);
    printf("입고 날자 : %s", startDate);
    free(startDate);

    // 날짜를 console 에서 입력 받는다
    time_t inputDate = inputDateFromConsole("유통기한");
    if (inputDate == -1) {
        printf("\n 날자 입력에 실패 하였습니다, 아무 키나 누르시면 돌아갑니다");
        _getch();
        return 1;
    }

    // 입력 받은 날자를 출력
    newItem.expire_date = inputDate;
    char* expDate = getFormatedStringByTime_t(&newItem.expire_date);
    printf("입력한 Expire date : %s\n", expDate);
    free(expDate);

    // 실제로 data를 reservedItem 에 저장 시도
    if (addReservedItem(&newItem) == 0) { printf("Error!!! fail to add ReservedItem()"); exit(0); }

    printf("\n입고 날자는 자동 저장 되지만 수정하고 싶으시면 메인 메뉴의 수정 메뉴를 이용하세요");
    printf("\n확인 하셨으면 y 키를 누르세요 >>");
    while (_getch() != 'y') {};
    if (doSaveToFile() == 0) { printf("Error !!! doSaveToFile()"); exit(1); }
    return 1;
}

// 새로운 아이템을  reservedItem 에 더한다
// 실패하면 0 를 retuen
int addReservedItem(_ITEM* item) {
    // tempItem에 현재 보다 하나 한개 더 메모리 확보
    //초기 생성 시 malloc, 추가 시 realloc
    if (reservedItem) reservedItem = (_ITEM*)realloc(reservedItem, (countOfItems + 1) * sizeof(_ITEM));
    else reservedItem = (_ITEM*)malloc(sizeof(_ITEM));

    if (reservedItem == 0) {
        printf("Error!!! cannot allocate for memory to tempItems");
        return 0;
    }
    reservedItem[countOfItems] = *item;
    countOfItems++;
    return 1;
}

// 하나의 항목을 제거하는 함수
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int doDeleteItem() {
    clearConsole();
    printf("*****************************************\n");
    printf("삭제할 품목의 번호를 선택하고 엔터를 누르세요\n");
    printf("*****************************************\n");

    printReservedItem();

    int select;
    int countOfTry = 0;

    // 지울 번호를 입력 받는다.
    while (1) {
        if (countOfTry == 3) {
            printf("\n3번 이상 입력에 실패했습니다, 삭제 실패, 아무키나 누르시면 돌아 갑니다");
            return 1;
        }

        printf("\n지울 아이템의  번호를 입력하세요  종료 하시려면 999 >> ");
        if (scanf("%d", &select) == 0) {
            countOfTry++;
            continue;
        }

        if (select == 999) return 1;
        if (select <= 0 || select > countOfItems) {
            printf("잘못된 번호입니다\n");
            countOfTry++;
            continue;
        }

        break;
    }


    printf("\n 지울 항목 %d 맞으면 y,  아니라면  n 을 입력", select);
    while (1) {
        char answer = _getch();
        if (answer == 'y') {
            if (removeFromReservedItem(select-1) == 0) exit(1);   // 실제 제거하는 함수
            break;
        }
        else if (answer == 'n') break;
    }
    if (doSaveToFile() == 0) { printf("Error !!! doSaveToFile()"); exit(1); }
    return 1;
}

// tempItems에 countOfItems 보다 하나 적은 메모리를 확보 후 
// reservedItem으로 for loop 돌면서 지울 내용이 아니면 tempItems 에 복사한다
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int removeFromReservedItem(int deleteNo) {
    if (deleteNo < 0 || deleteNo >= countOfItems) {
        printf("Error!!! removeFromReservedItem = 잘못된  index를 사용\n");
        return 0;
    }
    else {
        // 새롭게 저장할 포인터
        _ITEM* tempItems;

        // 메모리 확보
        tempItems = (_ITEM*)malloc((countOfItems - 1) * sizeof(_ITEM));
        if (tempItems == NULL) {
            printf("Error!!! removeFromReservedItem = 메모리 확보 실패, 아무키나 누르시면 돌아갑니다\n");
            _getch();
            return 0;
        }

        // for loop를 돌면서 지울 항목을 제외하고는 복사
        int savePosition = 0;  //tempItems에서 사용할 배열 첨자 변수
        for (int i = 0; i < countOfItems; i++) {
            // 지울 항목이면 pass
            if (i == deleteNo) continue;

            tempItems[savePosition] = reservedItem[i];

            savePosition++;
        }

        // 성공했으면 count  하나 빼줌
        countOfItems--;

        // 메모리를 해제하고 tempItems로 교체
        free(reservedItem);
        reservedItem = tempItems;

        return 1;
    }
}

// 저장된  data를 수정하는 함수, 실제 수정은 modifyReservedItem 에서 실행됨
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int doModifyItem() {
    clearConsole();
    printReservedItem();

    if (countOfItems == 0) {
        printf("\n 저장된 품목이 없습니다 되돌아가시려면 아무 키나 누르세요 >>");
        _getch();
        return 1;
    }

    int select;
    int countOfTry = 0;

    while (1) {
        if (countOfTry == 3) return 0;
        printf("\n수정할 아이템의  번호를 입력하세요,  그냥 종료하려면 999>> ");
        if (scanf("%d", &select) == 0) {
            countOfTry++;
            continue;
        }
        if (select == 999) return 1;
        else if (select <= 0 || select > countOfItems) {
            printf("잘못된 번호입니다\n");
            countOfTry++;
            continue;
        }
        break;
    }
    printf("\n 수정할 항목이  [[ %d ]]  맞으면 y,  아니라면  n 을 입력 >>", select);
    while (1) {
        char answer = _getch();
        if (answer == 'y') {
            if (modifyReservedItem(select-1) == 1) {
                if (doSaveToFile() == 0) { printf("Error !!! doSaveToFile()"); exit(1); }
                return 1;
            }// 실제 수정하는 함수를 실행
            else {
                printf("Error!!! modifyReservedItem  실행중 error 발생");
                return 0;
            }
        }
        else if (answer == 'n') break;

    }
}

// 실제 수정하는 부분
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int modifyReservedItem(int select) {

    int retry = 0;
    char selectOption;

    while (1) {
        clearConsole();

        printReservedItem();

        printf("\n수정할 항목에 따라 단축기를 누르세요 : 품목이름-p,   개수(단위는 빼고 입력하세요)-c,  입고날자-s, 유통기한-e, 종료-q >> ");
        if (retry == 3) {
            printf("\nWarning!!! 3번 이상 잘 못 입력하셨습니다\n");
            _getch();
            return 1;
        }

        selectOption = _getch();
        switch (selectOption) {
            // Enter key를 누르면 \n이 남으므로 buffer를 비움
            while (getchar() != '\n');
        case 'p':
        {
            char newName[20];
            printf("\n품목의 이름을   입력 하세요 19자 이하>> ");
            scanf("%19s", newName, (unsigned)_countof(newName));
            newName[19] = '\0'; // 19자 보다 길게 입력하면 overflow가 생기므로 대비 위해 인위적으로문자열 끝을 추가해 주었다.
            strcpy(reservedItem[select].name, newName);
            retry = 0;
            break;
        }
        case 'c':
        {
            int newCount;
            printf("\n품목의 개수를    입력 하세요 >> ");
            scanf("%d", &newCount);
            reservedItem[select].count = newCount;
            retry = 0;
            break;
        }
        case 's':
        {
            time_t newStartDate;
            newStartDate = inputDateFromConsole("새로운 입고 날자");
            if (newStartDate == -1) printf("\n날자 입력에 실패하셧어요, 다시 시도 하세요");
            else reservedItem[select].start_date = newStartDate;
            retry = 0;
            break;
        }
        case 'e':
        {
            time_t newExpireDate;
            newExpireDate = inputDateFromConsole("새로운 유통 기한");
            if (newExpireDate == -1) printf("\n날자 입력에 실패하셧어요, 다시 시도 하세요");
            else reservedItem[select].expire_date = newExpireDate;
            retry = 0;
            break;
        }
        case 'q':
            return 1;
        default:
            retry++;
            continue;
        }
    }

    return 1;

}


// 냉장고 입고 품목을 프린트 하는 함수
int printReservedItem() {
    printf("\n------------------------------------------------------------------------------------------------\n");
    printf("%5s  %-20s  %-5s  %-30s  %-30s\n", "번호", "   품목 이름", "갯수", "입고날자", "유통기한");
    printf("------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < countOfItems; i++) {
        char* start_date = getFormatedStringByTime_t(&reservedItem[i].start_date);
        char* expire_date = getFormatedStringByTime_t(&reservedItem[i].expire_date);
        printf("%5d  %-20s  %3d개  %-30s  %-30s\n", i+1, reservedItem[i].name, reservedItem[i].count, start_date, expire_date);
        free(start_date);  //  getFormatedStringByTime_t에서 malloc으로 할당한 메모리 해제
        free(expire_date); //  getFormatedStringByTime_t에서 malloc으로 할당한 메모리 해제
    }
    printf("------------------------------------------------------------------------------------------------\n");

    return 1;
}

// time_t => YYYY년 MM월 DD일 로 변환
// caller가 buffer를 free 해 주어야 한다
// 실패하면 NULL을 return
char* getFormatedStringByTime_t(time_t* ttCurrent) {

 
    struct tm localTM;
    localtime_s(&localTM, ttCurrent);
    char* buff = (char*)malloc(sizeof(char) * 256);

    if (buff == 0) {
        printf("Memory allocation error in getFormatedStringByTime_t,  아무키나 입력하세요\n");
        _getch();
        return NULL;
    }
    else {

        if (strftime(buff, sizeof(char) * 256, "%Y년 %m월 %d일", &localTM) == 0) {
            printf("time_t를 문자열로 변경하는데 실패했습니다, 아무 키나 입력하세요 \n");
            _getch();
            free(buff);
            return NULL;
        }

        // 끝에 포함된 \n을 없앤다.
        //size_t length = strlen(buff);
        //if (length > 0 && buff[length - 1] == '\n') {
        //    buff[length - 1] = '\0';
        //}

        return buff;
    }
}

// 실패하면 0 를 retuen
int getCursorPosition(_CURSOR_POS* cursor_pos) {
    CONSOLE_SCREEN_BUFFER_INFO presentCur;

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur) == 0) {
        printf("Error !!! GetConsoleScreenBufferInfo");
        return 0;
    }
    else {
        cursor_pos->x = presentCur.dwCursorPosition.X;
        cursor_pos->y = presentCur.dwCursorPosition.Y;
        //printf("current cursor position  = %d, %d\n",cursor_pos->x, cursor_pos->y);
        return 1;
    }
}


// 실패하면 0 를 retuen
int setCursorPosition(_CURSOR_POS* cursor_pos) {
    COORD newPosition = { (SHORT)cursor_pos->x, (SHORT)cursor_pos->y };
    if (SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), newPosition) == 0) {
        printf("Error in Setting Cursor to New Postion  = %d, %d\n", cursor_pos->x, cursor_pos->y);
        return 0;
    }
    else {
        return 1;
    }
}

// 실패하면 0 를 retuen
int clearConsole() {
    if (system("cls")) return 1;
    else return 0;
}

void setTextHighlight() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
}

void setTextNormal() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
}



// console로 부터 연, 월 일을 입력받아 검증 후 해당 time_t값을  return
// 실패하면 -1을 return
time_t inputDateFromConsole(const char* message) {
    // 기본 tm 구조 선언
    struct tm inputTime = { 0 };
    time_t retTime_t;

    //  data  입력
    printf("\n%s을(를) 다음 형식과 같이 입력하세요 (YYYY-MM-DD) >> ", message);
    int retry = 0;
    while (1) {
        if (retry == 3) {
            printf("\n 3회 이상 입력에 실패 했습니다, 아무키나 누르시면 되돌아 갑니다");
            _getch();
            return -1;
        }
        else if (scanf("%d-%d-%d", &inputTime.tm_year, &inputTime.tm_mon, &inputTime.tm_mday) != 3) {
            printf("\n정확한 날짜 정보를 입력하세요 e.g 2023-11-14 Error %d회\n", retry);
            retry++;
            while (getchar() != '\n'); // input buffer를 비운다
            continue;
        }
        else {
            inputTime.tm_year -= 1900;
            inputTime.tm_mon--;
            retTime_t = mktime(&inputTime);
            if (retTime_t == -1) {
                printf("\n입력하신 날자가 유효한 날자가 아닙니다.  다시 입력하세요\n");
                retry++;
                continue;
            }
            else {
                return retTime_t;
            }
        }
    }
}

// 파일에 _ITEM* 저장
// 실패하면 message 보여주고 0를  return
int doSaveToFile() {
    // binary 쓰기 모드
    FILE* filePointer;
    filePointer = fopen(strFileName, "wb");

    if (filePointer == NULL) {
        printf("file open에 실패했습니다,  프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 0;
    }

    // fwrite ( data에 대한 pointer, size of item, count of item, file pointer )
    // countOfItem 을 쓴다 
    if (fwrite(&countOfItems, sizeof(int), 1, filePointer) != 1) {
        fclose(filePointer);
        printf("file 쓰기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 0;
    }

    // reservedItem 을 쓴다, 
    if (fwrite(reservedItem, sizeof(_ITEM), countOfItems, filePointer) != countOfItems) {
        fclose(filePointer);
        printf("file 쓰기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 0;
    }

    fclose(filePointer);

    // 성공 메시지 표시

    return 1;
}

// 파일에서 불러오기
// 쓸때 차지한  data를 잘 생각해서 복원
// 실패하면 message 보여주고 0를  return
int doReadFromFile() {
    FILE* filePointer;
   

    if ((filePointer = fopen(strFileName, "rb")) ==NULL) {
 //       filePointer = fopen(strFileName, "wb");
 //       fwrite(&countOfItems, sizeof(int), 1, filePointer);//음식 데이터를 저장한 파일이 없을 시 파일 만들기 후 return //이때 countOfItems = 0
 //       fclose(filePointer);
        return 1;
    }

    // 항목이 몇개 저장되어있는지 읽어오기
    if (fread(&countOfItems, sizeof(int), 1, filePointer) != 1) {
        fclose(filePointer);
        printf("\nfile 읽기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 0;
    }
    //reserveditem 메모리 확보
    reservedItem = (_ITEM*)malloc(sizeof(_ITEM) * countOfItems);
    if (reservedItem == 0) {
        printf("\n memory 확보에 실패 했습니다. 프로그램 제작자와 상의하세요 아무키나 누르세요");
        _getch();
        return 0;
    }

    // 파일에서 데이터 읽어와서 reserveditem에 저장
    if (fread(reservedItem, sizeof(_ITEM), countOfItems, filePointer) != countOfItems) {
        fclose(filePointer);
        printf("\nfile 읽기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        free(reservedItem);
        return 0;
    }
    

    fclose(filePointer);

    // 성공 메시지 표시
//    printf("\n파일에서 데이터를 성공적으로 불러 왔습니다. 아무키나 누르세요 >>");
//    _getch();

    return 1;
}


int compareItemsByNameASC(const _ITEM* a, const _ITEM* b) {
    return strcmp(a->name, b->name);
}

int compareItemsByNameDESC(const _ITEM* a, const _ITEM* b) {
    return strcmp(b->name, a->name);
}

int compareItemsByStartDateASC(const _ITEM* a, const _ITEM* b) {
    return (a->start_date > b->start_date);
}

int compareItemsByStartDateDESC(const _ITEM* a, const _ITEM* b) {
    return (a->start_date < b->start_date);
}

int compareItemsByExpireDateASC(const _ITEM* a, const _ITEM* b) {
    return (a->expire_date > b->expire_date);
}

int compareItemsByExpireDateDESC(const _ITEM* a, const _ITEM* b) {
    return (a->expire_date < b->expire_date);
}

void displayRecipeMenu(int selected) { // 메인메뉴입니다.
    system("cls");
    draw_line2(60); printf("\n");
    printf("레시피 메뉴입니다.\n방향키를 통해 원하는 메뉴를 선택한 후 Enter를 누르거나\n실행하고자 하는 메뉴의 숫자를 입력하세요.\n\n");
    draw_line2(60); printf("\n");
    printf("1. 레시피 조회 %s\n", (selected == 1) ? ANSI_COLOR_RED" <-"ANSI_COLOR_RESET : "");
    printf("2. 레시피 추가 %s\n", (selected == 2) ? ANSI_COLOR_RED" <-"ANSI_COLOR_RESET : "");
    printf("3. 레시피 수정 %s\n", (selected == 3) ? ANSI_COLOR_RED" <-"ANSI_COLOR_RESET : "");
    printf("4. 레시피 삭제 %s\n", (selected == 4) ? ANSI_COLOR_RED" <-"ANSI_COLOR_RESET : "");
    printf("5. 이전화면으로  %s\n", (selected == 5) ? ANSI_COLOR_RED" <-"ANSI_COLOR_RESET : "");
    printf("\n"); draw_line2(60);
}

int doRecipePage() {
    
    int mainChoice = 1;
    char key;

    while (1) {
        displayRecipeMenu(mainChoice);

        key = getch();

        switch (key) {
        case ARROW_UP:  // 방향키 위
            mainChoice = (mainChoice - 1 > 0) ? mainChoice - 1 : 5;
            break;
        case ARROW_DOWN:  // 방향키 아래
            mainChoice = (mainChoice + 1 <= 5) ? mainChoice + 1 : 1;
            break;
        case '1'://숫자키 입력으로 메뉴 바로가기
            doPrintRecepi();
            break;
        case '2':
            doAddNewRecipe();
            break;
        case '3':

            break;
        case '4':
            doDeleteRecipe();
            break;
        case '5':
            return 1;
            break;
        case '\r':  // Enter 키
            switch (mainChoice) {
            case 1: // 레시피 조회                
                doPrintRecepi();
                break;

            case 2: // 레시피 추가               
                doAddNewRecipe();
                break;

            case 3: // 레시피 수정 //미완성                
                draw_line2(60); printf("\n");
                printf("레시피 수정 메뉴입니다.\n수정하고 싶은 레시피 번호를 입력해주세요.\n수정을 원치 않는 경우 999를 입력해주세요.\n\n");
                draw_line2(60); printf("\n");
                printf("이 곳에 레시피 목록이 출력됩니다.\n\n");
                draw_line2(60); printf("\n");

                break;


            case 4: // 레시피 삭제               
                doDeleteRecipe();
                break;

            case 5: //이전화면으로               
                return 1;
            }
            break;
        }

       
    } 
    return 0;
}

void draw_line1(int line_length)
{
    int i;
    for (i = 0; i < line_length; i++)
        printf("-");
    printf("\n");
}

void draw_line2(int line_length)
{
    int i;
    for (i = 0; i < line_length; i++)
        printf("=");
    printf("\n");
}


/*
*************************************************************************************************************
*/

void doPrintRecepi() {
    printReservedRecipe();
    printf("------------------------------------------------------------------------------------------------\n");
    printf("\n확인 하셨으면 y 키를 누르세요 >>");
    while (tolower(_getch()) != 'y') {};
    if (doRecipeSaveToFile() == 0) { printf("Error !!! doSaveToFile()"); exit(1); }
}

void recipe_init(_RECIPE_ITEM* newRecipeItem) //레시피 전체의 음식 이름은 "", 개수는 0으로 전부 초기화됩니다.
{
    for (int i = 0; i < FOOD_NUMBER_MAX; i++)
    {
        strcpy(newRecipeItem[i].name, "");
        newRecipeItem[i].count = 0;
    }
      
}

// 새로운 항목을 추가 해 줌 - 실제 기능은 addReservedRecipe() 함수가 시행
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int doAddNewRecipe() {
    _RECIPE newRecipe;
    clearConsole();
    printf("추가할 레시피의 이름을 입력하세요 \n");
    scanf("%s", newRecipe.name);
    // char name[20] '\0' 를 위해 19자만 입력 가능

    recipe_init(&newRecipe.food);
    char c;
    int i;
    for (i = 0;i < FOOD_NUMBER_MAX; i++) {
        printf("%d번 재료 이름 : (19자이내) >> ",i+1);
        scanf("%s", newRecipe.food[i].name);
        printf("재료 개수(단위는 빼고 입력하세요) >> ");
        scanf("%d", &newRecipe.food[i].count);

        if (i < FOOD_NUMBER_MAX - 1) {
            printf("더 추가할 재료가 있습니까(있으면 Y(y), 없으면 다른 키 입력) ");
        }
        else {
            printf("재료는 %d개까지만 추가할 수 있습니다.", FOOD_NUMBER_MAX);
            break;
        }
        while (getchar() != '\n');
        if (toupper(getchar()) == 'Y') {
            while (getchar() != '\n');
            continue;
        }
        else {
            break;
        }
    }
    

    // 실제로 data를 reservedItem 에 저장 시도
    if (addReservedRecipe(&newRecipe) == 0) { printf("Error!!! fail to add ReservedItem()"); exit(0); }

    printf("\n확인 하셨으면 y 키를 누르세요 >>");
    while (tolower(_getch()) != 'y') {};
    if (doRecipeSaveToFile() == 0) { printf("Error !!! doSaveToFile()"); exit(1); }
    return 1;
}

// 새로운 아이템을  reservedItem 에 더한다
// 실패하면 0 를 retuen
int addReservedRecipe(_RECIPE* item) {
    // tempItem에 현재 보다 하나 한개 더 메모리 확보
    //초기 생성 시 malloc, 추가 시 realloc
    if (recipe) recipe = (_RECIPE*)realloc(recipe, (countOfRecipe + 1) * sizeof(_RECIPE));
    else recipe = (_RECIPE*)malloc(sizeof(_RECIPE));

    if (recipe == 0) {
        printf("Error!!! cannot allocate for memory to tempItems");
        return 0;
    }
    recipe[countOfRecipe] = *item;
    countOfRecipe++;
    return 1;
}







// 하나의 항목을 제거하는 함수
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int doDeleteRecipe() {
    
    printf("*****************************************\n");
    printf("삭제할 품목의 번호를 선택하고 엔터를 누르세요\n");
    printf("*****************************************\n");

    printReservedRecipe();

    int select;
    int countOfTry = 0;

    // 지울 번호를 입력 받는다.
    while (1) {
        if (countOfTry == 3) {
            printf("\n3번 이상 입력에 실패했습니다, 삭제 실패, 아무키나 누르시면 돌아 갑니다");
            return 1;
        }

        printf("\n지울 아이템의  번호를 입력하세요  종료 하시려면 999 >> ");
        if (scanf("%d", &select) == 0) {
            countOfTry++;
            continue;
        }

        if (select == 999) return 1;
        if (select <= 0 || select > countOfRecipe) {
            printf("잘못된 번호입니다\n");
            countOfTry++;
            continue;
        }

        break;
    }


    printf("\n 지울 항목 %d 맞으면 y,  아니라면  n 을 입력", select);
    while (1) {
        char answer = _getch();
        if (answer == 'y') {
            if (removeFromReservedRecipe(select - 1) == 0) exit(1);   // 실제 제거하는 함수
            break;
        }
        else if (answer == 'n') break;
    }
    if (doRecipeSaveToFile() == 0) { printf("Error !!! doSaveToFile()"); exit(1); }

    printf("\n삭제 완료했습니다\n확인 하셨으면 y 키를 누르세요 >>");
    while (tolower(_getch()) != 'y') {};
    if (doRecipeSaveToFile() == 0) { printf("Error !!! doSaveToFile()"); exit(1); }
    return 1;
    return 1;
}

// tempItems에 countOfItems 보다 하나 적은 메모리를 확보 후 
// reservedItem으로 for loop 돌면서 지울 내용이 아니면 tempItems 에 복사한다
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int removeFromReservedRecipe(int deleteNo) {
    if (deleteNo < 0 || deleteNo >= countOfRecipe) {
        printf("Error!!! removeFromReservedItem = 잘못된  index를 사용\n");
        return 0;
    }
    else {
        // 새롭게 저장할 포인터
        _RECIPE* tempItems;

        // 메모리 확보
        tempItems = (_RECIPE*)malloc((countOfRecipe - 1) * sizeof(_RECIPE));
        if (tempItems == NULL) {
            printf("Error!!! removeFromReservedItem = 메모리 확보 실패, 아무키나 누르시면 돌아갑니다\n");
            _getch();
            return 0;
        }

        // for loop를 돌면서 지울 항목을 제외하고는 복사
        int savePosition = 0;  //tempItems에서 사용할 배열 첨자 변수
        for (int i = 0; i < countOfRecipe; i++) {
            // 지울 항목이면 pass
            if (i == deleteNo) continue;

            tempItems[savePosition] = recipe[i];

            savePosition++;
        }

        // 성공했으면 count  하나 빼줌
        countOfRecipe--;

        // 메모리를 해제하고 tempItems로 교체
        free(recipe);
        recipe = tempItems;

        return 1;
    }
}




// 레시피를 프린트 하는 함수
int printReservedRecipe() {
    clearConsole();
    printf("\n------------------------------------------------------------------------------------------------\n");
    printf("레시피");
    printf("\n----------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < countOfRecipe; i++) {
        printf("%d번  %-15s\n", i + 1, recipe[i].name);
        for (int j = 0; j < FOOD_NUMBER_MAX; j++) {
            if (strcmp(recipe[i].food[j].name, "") == 0) continue;
            else printf("%-20s  %3d개               ", recipe[i].food[j].name, recipe[i].food[j].count);
        }
        printf("\n");

    }
    
    

    return 1;
}




// 파일에 _ITEM* 저장
// 실패하면 message 보여주고 0를  return
int doRecipeSaveToFile() {
    // binary 쓰기 모드
    FILE* filePointer;
    filePointer = fopen(strRecipeFileName, "wb");

    if (filePointer == NULL) {
        printf("file open에 실패했습니다,  프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 0;
    }

    // fwrite ( data에 대한 pointer, size of item, count of item, file pointer )
    // countOfItem 을 쓴다 
    if (fwrite(&countOfRecipe, sizeof(int), 1, filePointer) != 1) {
        fclose(filePointer);
        printf("file 쓰기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 0;
    }

    // reservedItem 을 쓴다, 
    if (fwrite(recipe, sizeof(_RECIPE), countOfRecipe, filePointer) != countOfRecipe) {
        fclose(filePointer);
        printf("file 쓰기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 0;
    }

    fclose(filePointer);

    // 성공 메시지 표시

    return 1;
}

// 파일에서 불러오기
// 쓸때 차지한  data를 잘 생각해서 복원
// 실패하면 message 보여주고 0를  return
int doRecipeReadFromFile() {
    FILE* filePointer;


    if ((filePointer = fopen(strRecipeFileName, "rb")) == NULL) {
//        filePointer = fopen(strRecipeFileName, "wb");
//        fwrite(&countOfRecipe, sizeof(int), 1, filePointer);//음식 데이터를 저장한 파일이 없을 시 파일 만들기 후 return //이때 countOfRecipe = 0
//        fclose(filePointer);
        return 1;
    }

    // 항목이 몇개 저장되어있는지 읽어오기
    if (fread(&countOfRecipe, sizeof(int), 1, filePointer) != 1) {
        fclose(filePointer);
        printf("\nfile 읽기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 0;
    }
    //reserveditem 메모리 확보
    recipe = (_RECIPE*)malloc(sizeof(_RECIPE) * countOfRecipe);
    if (recipe == 0) {
        printf("\n memory 확보에 실패 했습니다. 프로그램 제작자와 상의하세요 아무키나 누르세요");
        _getch();
        return 0;
    }

    // 파일에서 데이터 읽어와서 reserveditem에 저장
    if (fread(recipe, sizeof(_RECIPE), countOfRecipe, filePointer) != countOfRecipe) {
        fclose(filePointer);
        printf("\nfile 읽기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        free(recipe);
        return 0;
    }


    fclose(filePointer);

    // 성공 메시지 표시
//    printf("\n파일에서 데이터를 성공적으로 불러 왔습니다. 아무키나 누르세요 >>");
//    _getch();

    return 1;
}
