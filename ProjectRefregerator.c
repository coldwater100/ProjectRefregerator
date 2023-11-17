
#include <stdio.h>
#include <time.h>
#include<Windows.h>
#include <conio.h>
#include <stdlib.h>

#define ARROW_DOWN 80
#define ARROW_UP 72
#define ARROW_LEFT 75
#define ARROW_RIGHT 77
#define COUNT_OF_MAIN_MENU 7

#define NOT_SORTED 101
#define ASC_SORTED 102
#define DESC_SORTED 103

// 보관 물품 항목에 대한 구조체
typedef struct {
    char name[20];
    int count;
    time_t start_date;
    time_t expire_date;
} _ITEM;

// 커서 포지션에 대한 구조체
typedef struct {
    int x;
    int y;
} _CURSOR_POS;

// main menu의 cursor 위치를 저장할 변수
_CURSOR_POS menuPosition[COUNT_OF_MAIN_MENU];

// main menu의 enum 상수
enum MENU_ITEM {
    CURRENT_STATUS, REMOVE_FOOD, NEW_FOOD, MODIFY_ITEM, READ_FROM_FILE, WRITE_TO_FILE, EXIT
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
int doModifyItem(); // 수정 기초 작업
int modifyReservedItem(int select); // 실제 수정 기능

// 저장된 file에서 불러 오기 및 쓰기
int doReadFromFile();
int doSaveToFile();

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


// 기본 화면에 표시할 메뉴 문자열
const char* MENU[] = {
    "0 >> 현재 상황 및 음식 출력\n",
    "1 >> Remove Food\n",
    "2 >> New Food\n",
    "3 >> Modify Item\n",
    "4 >> File에서 불러오기\n",
    "5 >> File에 현상태 저장하기\n",
    "6 >> Exit\n"
};


// 현재 냉장고 보관 물품 목록
_ITEM* reservedItem;

// 현재 냉장고 보관 품목 수
//reservedItem과 항상 동기를 정확히 시켜주어야 함
int countOfItems = 0;

// 냉장고 온도
int refregiratorTemperature = 5;
int freezerTemperature = -5;


int main()
{

    printMenu();

    char arrow; // arrow를 누르면 224, ARROW_DOWN, ARROW_UP 값이 순차적으로 들어 옴

    // arrow key이면 selectedMenu를 바꾸고,  enter key이면  현재의 selectedMenu를 실행 시킴
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
    printf("커서를 선택할 메뉴로 이동한 뒤 엔터를 누르세요\n\n");
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
    case READ_FROM_FILE:
        if (doReadFromFile() == 0) { printf("Error !!! doReadFromFile()"); exit(1); }
        break;
    case WRITE_TO_FILE:
        if (doSaveToFile() == 0) { printf("Error !!! doSaveToFile()"); exit(1); }
        break;
    case EXIT:
        exit(0);
    default:break;
    }
}

// 현재 냉장고의 상태와 전체 보관 항목을 표시해 주는 역할
void doListItems() {

    // 현재 선택된 메뉴 설정
    enum STATUS_BUTTON_MENU currentSelection = PREVIOUS;

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
    printf("\n냉장고 관리 시스템");
    char* strCurrentTime = getFormatedStringByTime_t(&currentTime_t);
    printf("\n%s", strCurrentTime);
    free(strCurrentTime);

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
        printf("%3d      %-20s  %3d개     %-20s  %-20s\n", i, reservedItem[i].name, reservedItem[i].count, start_date, expire_date);
        free(start_date);
        free(expire_date);
    }
    printf("------------------------------------------------------------------------------------------------\n");

    // 이전화면으로 버튼 표시
    if (currentSelection == PREVIOUS) setTextHighlight();
    printf("\n[이전화면으로]\n");
    setTextNormal();

    printf("\n상 하 좌 우  화살표 키로 선택할 메뉴로 이동한 뒤 엔터키를 누르세요\n");
    printf("\n네모 세모 버튼 선택 후 엔터키를 누르면 정렬 순서를 바꿀 수 있습니다\n");
}

// 새로운 항목을 추가 해 줌 - 실제 기능은 addReservedItem() 함수가 시행
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int doAddNewItem() {
    _ITEM newItem;

    clearConsole();
    printf("보관할 새로운 정보를 입력합니다 \n\n");
    // char name[20] '\0' 를 위해 19자만 입력 가능
    printf("품목 이름 : (19자이내) >> ");
    scanf_s("%s", newItem.name, (unsigned)_countof(newItem.name));
    printf("품목 갯수 >> ");
    scanf_s("%d", &newItem.count);

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

    return 1;
}

// 새로운 아이템을  reservedItem 에 더한다
// tempItems에 메모리 확보후   reservedItems의 내용을 복사후  reservedItem 의 주소값을 tempItems의 주소값으로 대체
// 실패하면 0 를 retuen
int addReservedItem(_ITEM* item) {
    // tempItem에 현재 보다 하나 한개 더 메모리 확보
    _ITEM* tempItems;
    tempItems = (_ITEM*)malloc((countOfItems + 1) * sizeof(_ITEM));

    // tempItems에 메모리가 확보 되었으면 reservedItems를 복사하고
    // reservedItem의  내용을 복사
    // reservedItem으로 잡힌 memory 해제후 tempItems의 주소 복사
    if (tempItems == 0) {
        printf("Error!!! cannot allocate for memory to tempItems");
        return 0;
    }
    else {
        memcpy(tempItems, reservedItem, countOfItems * sizeof(_ITEM));
        free(reservedItem);
        reservedItem = tempItems;
    }

    // reservedItem 의 마지막에 현재 item을 붙이고 카운트 증가 시켜줌
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
        if (scanf_s("%d", &select) == 0) {
            countOfTry++;
            continue;
        }

        if (select == 999) return 1;

        if (select < 0 || select >= countOfItems) {
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
            if (removeFromReservedItem(select) == 0) exit(1);   // 실제 제거하는 함수
            break;
        }
        else if (answer == 'n') break;
    }

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

// 저장된  data를 수정하는 함수, 실제 수정은 modifyReservedItem 에서 실행ㄷ됨
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
        if (scanf_s("%d", &select) == 0) {
            countOfTry++;
            continue;
        }
        if (select == 999) return 1;
        else if (select < 0 || select >= countOfItems) {
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
            if (modifyReservedItem(select) == 1) return 1;     // 실제 수정하는 함수를 실행
            else {
                printf("Error!!! modifyReservedItem  실행중 error 발생");
                return 0;
            }
        }
        else if (answer == 'n') break;

    }

    return 1;
}

// 실제 수정하는 부분
// data 무결성에 영향을 미치면 0를  return, 아니면 1를 return
int modifyReservedItem(int select) {

    int retry = 0;
    char selectOption;

    while (1) {
        clearConsole();

        printReservedItem();

        printf("\n수정할 항목에 따라 단축기를 누르세요 : 품목이름-p,   갯수-c,  입고날자-s, 유통기한-e, 종료-q >> ");
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
            scanf_s("%19s", newName, (unsigned)_countof(newName));
            newName[19] = '\0'; // 19자 보다 길게 입력하면 overflow가 생기므로 대비 위해 인위적으로문자열 끝을 추가해 주었다.
            strcpy_s(reservedItem[select].name, 20, newName);
            retry = 0;
            break;
        }
        case 'c':
        {
            int newCount;
            printf("\n품목의 갯수를    입력 하세요 >> ");
            scanf_s("%d", &newCount);
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
        printf("%5d  %-20s  %3d개  %-30s  %-30s\n", i, reservedItem[i].name, reservedItem[i].count, start_date, expire_date);
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



// console로 부터 time_t구조를 입력 받아 검정 받은 후 return
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
        else if (scanf_s("%d-%d-%d", &inputTime.tm_year, &inputTime.tm_mon, &inputTime.tm_mday) != 3) {
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
    clearConsole();
    printf("---------------------------------------------\n");
    printf("현재 상태를 파일로 저장\n");
    printf("---------------------------------------------\n");

    printf("현재 메모리 상의 데이터 입니다\n");
    printReservedItem();

    printf("\n 현재 상태를 파일로 저장합니다,   이전 저장 파일은 없어집니다  \n\n ** 파일에 쓰시려면 y 취소하려면 아무 키 입력 >>");
    char answer = _getch();
    if (answer != 'y') return 1;

    // binary 쓰기 모드
    FILE* filePointer;
    fopen_s(&filePointer, strFileName, "wb");

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
    printf("\n파일에서 데이터를 저장했습니다. 아무키나 누르세요 >>");
    _getch();

    return 1;
}

// 파일에서 불러오기
// 쓸때 차지한  data를 잘 생각해서 복원
// 실패하면 message 보여주고 0를  return
int doReadFromFile() {
    clearConsole();
    printf("---------------------------------------------\n");
    printf("파일에서 불러 오기\n");
    printf("---------------------------------------------\n");

    printf("현재 메모리 상의 데이터 입니다\n");
    printReservedItem();

    printf("\n 파일에서 냉장고 데이터를 읽어 옵니다,  현재 메모리 상의 데이터는 없어집니다  \n\n ** 읽어 오려면 y 취소하려면 아무 키 입력 >>");
    char answer = _getch();
    if (answer != 'y') return 1;

    FILE* filePointer;

    if (fopen_s(&filePointer, strFileName, "rb") != 0) {
        printf("\nfile 열기에 실패했습니다, 저장된 파일이 있는지 확인해 보세요, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 1;
    }

    // Read the count of items from the file
    if (fread(&countOfItems, sizeof(int), 1, filePointer) != 1) {
        fclose(filePointer);
        printf("\nfile 읽기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        return 0;
    }

    // Allocate memory for reservedItem based on the count read from the file
    free(reservedItem);
    reservedItem = (_ITEM*)malloc(sizeof(_ITEM) * countOfItems);
    if (reservedItem == 0) {
        printf("\n memory 확보에 실패 했습니다. 프로그램 제작자와 상의하세요 아무키나 누르세요");
        _getch();
        return 0;
    }

    // Read reservedItem from the file
    if (fread(reservedItem, sizeof(_ITEM), countOfItems, filePointer) != countOfItems) {
        fclose(filePointer);
        printf("\nfile 읽기에 실패했습니다, 프로그램 제작자와 상의하세요,  아무 키나 누르세요");
        _getch();
        free(reservedItem);
        return 0;
    }

    fclose(filePointer);

    // 성공 메시지 표시
    printf("\n파일에서 데이터를 성공적으로 불러 왔습니다. 아무키나 누르세요 >>");
    _getch();

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