#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 300

//-------------DICHIARAZIONE STRUCT-------------
struct sheet{
    char* text;
    struct sheet* next;
    struct sheet* prev;
};

struct sheetList{
    int line;
    struct sheet* nil;
};

struct actionUndo{
    char action;
    int val;
    int val2;
    int lineBeforeCommand;
    struct sheet* node1;
    struct sheet* node2;
    struct actionUndo* prev;
};

struct stack{
    int top;
    struct actionUndo* node;
};

//--------------INIZIALIZATION STRUCT-----------------
void initializationTextlist (struct sheetList** list){
    (*list)->nil->next = (*list)->nil;
    (*list)->nil->prev = (*list)->nil;
    (*list)->line=0;
    strcpy((*list)->nil->text,"0");
}

void initializationStack(struct stack* listAction){
    listAction->top=0;
    listAction->node = NULL;
}

//---------------CREATE STRUCT-----------------
struct sheet* createNewNode(){
    struct sheet* node = (struct sheet*) malloc(sizeof(struct sheet));

    node->next=NULL;
    node->prev=NULL;
    return node;
}

struct actionUndo* createUndo(char action, int val, int val2, int numberAction, char** text, struct stack* listAction){
    struct actionUndo* node = (struct actionUndo*) malloc(sizeof(struct actionUndo));

    node->action=action;
    node->val=val;
    node->val2=val2;
    node->prev=listAction->node;
    return node;
}

//-------------METODI PER LISTE------------------

void deleteNode(struct sheet* node){
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
}

struct sheet* findNode(struct sheetList* list, int key){
    if(list->line==0 || key>list->line || key==0){
        return list->nil;
    }
    struct sheet* node = list->nil->next;
    for(int i=1; i<key; i++){
        node= node->next;
    }
    return node;
}

void deleteAllText(struct sheetList* list){
    struct sheet* node = list->nil->next;
    struct sheet* app;
    while(node!=list->nil && node!=NULL){
        app = node->next;
        free(node->text);
        deleteNode(node);
        node = app;
    }
}

//----------------METODI PER PILA----------------

void push(struct stack* listAction, char action, int val, int val2, struct sheet* node1, struct sheet* node2, int diffText){
    struct actionUndo* node = (struct actionUndo*) malloc(sizeof(struct actionUndo));

    node->action=action;
    node->val=val;
    node->val2=val2;
    node->prev=listAction->node;
    node->node1=node1;
    node->node2=node2;
    node->lineBeforeCommand = diffText;

    listAction->top = listAction->top+1;
    listAction->node = node;
}

struct actionUndo* pop(struct stack* listAction){
    if (listAction->top != 0)
        --listAction->top;
    struct actionUndo *action = listAction->node;
    listAction->node = action->prev;
    return action;
}


void deleteRedoList(struct stack* listAction){
    struct actionUndo* redo;
    while(listAction->top>0){
        redo = pop(listAction);
        if(redo!=NULL){

            if(redo->action=='e'){
                redo->node1=NULL;
                redo->node2=NULL;
            }
            free(redo);
        }
    }
    listAction->top = 0;
}

//---------------METODI PER UNDO E REDO-----------
void actionCanceled(struct actionUndo* action, struct sheetList* list, struct stack* listAction){
    struct sheet* nodeText1;
    struct sheet* nodeText2;

    switch(action->action){
        case 'i':{
            nodeText1=action->node1->prev;
            nodeText2=action->node2->next;

            nodeText1->next = action->node1;
            nodeText2->prev= action->node2;
            action->node1->prev = nodeText1;
            action->node2->next = nodeText2;

            push(listAction, 'e', action->val, action->val2, action->node1, action->node2, list->line);

            list->line = action->lineBeforeCommand;
            break;
        }
        case 'm':{
            nodeText1=action->node1->prev;
            nodeText2=action->node2->next;

            push(listAction, 'm', action->val, action->val2, nodeText1->next, nodeText2->prev, list->line);

            if(action->lineBeforeCommand!=0){
                list->line = action->lineBeforeCommand;
            }

            nodeText1->next=action->node1;
            action->node1->prev=nodeText1;
            nodeText2->prev=action->node2;
            action->node2->next=nodeText2;

            break;
        }
        case 'e':{
            if(action->val<=list->line){
                nodeText1 = action->node1->prev;
                nodeText2 = action->node2->next;

                nodeText1->next=nodeText2;
                nodeText2->prev=nodeText1;

                push(listAction, 'i', action->val, action->val2, action->node1, action->node2,list->line);

                list->line = action->lineBeforeCommand;

                break;
            } else {
                push(listAction, 's',0 , 0, NULL, NULL,0);
            }

        }
        case 's':{
            push(listAction, 's',0 , 0, NULL, NULL,0);
        }
    }
}

void executeUndo(struct sheetList* list, struct stack* undo, struct stack* redo, int numberActionExecute){
    int i;
    struct actionUndo* actionUndo =NULL;

    for(i=0; i<numberActionExecute && undo->top>0; i++){
        actionUndo = pop(undo);
        if(actionUndo!=NULL){
            actionCanceled(actionUndo, list, redo);
            free(actionUndo);
        }
    }
}

//------------------METODI PER C/D/P-------------
void create(struct sheetList* list, struct stack* undo, int val1, int val2){
    int i;
    char input_text[SIZE];
    struct sheet *nodeText1;
    struct sheet *nodeText2;
    struct sheet *nodeInsert1 = NULL;
    struct sheet *nodeInsert2 = NULL;

    //Creazione nuova lista da inserire
    struct sheet *newNode = createNewNode();
    nodeInsert1 = newNode;
    fgets(newNode->text, SIZE, stdin);
    for (i = val1 + 1; i <= val2; i++) {
        newNode->next = createNewNode();
        newNode->next->prev = newNode;
        newNode = newNode->next;
        fgets(newNode->text, SIZE, stdin);
    }
    nodeInsert2 = newNode;

    if (val1 == list->line + 1) {
        //Inserimento
        nodeText1 = list->nil->prev;
        nodeText1->next = nodeInsert1;
        nodeInsert1->prev = nodeText1;
        list->nil->prev = nodeInsert2;
        nodeInsert2->next = list->nil;


        push(undo, 'e', val1, val2, nodeInsert1, nodeInsert2, list->line);
        list->line += (val2 - val1 + 1);

    } else {
        //Modifica del testo

        nodeText1 = findNode(list, val1);
        nodeText2 = nodeText1;

        for (i = val1; i < val2; i++) {
            if (nodeText2->next == list->nil)
                break;
            nodeText2 = nodeText2->next;
        }

        nodeInsert1->prev = nodeText1->prev;
        nodeInsert2->next = nodeText2->next;
        nodeText1->prev->next = nodeInsert1;
        nodeText2->next->prev = nodeInsert2;

        push(undo, 'm', val1, val2, nodeText1, nodeText2, list->line);

        if (val2 > list->line) {
            list->line += val2-list->line;
        }


    }
}

void delete(struct sheetList* list, struct stack* undo, int val1, int val2){
    struct sheet *nodeDelete1;
    struct sheet *nodeDelete2;
    int i;

    if (val1 <= list->line && val2>0) {
        if(val1==0){
            val1++;
        }

        nodeDelete1 = findNode(list, val1);
        int num = 1;
        nodeDelete2 = nodeDelete1;
        for (i = val1; i < val2; i++) {
            if (nodeDelete2->next == list->nil)
                break;
            nodeDelete2 = nodeDelete2->next;
            num++;
        }
        nodeDelete1->prev->next = nodeDelete2->next;
        nodeDelete2->next->prev = nodeDelete1->prev;

        push(undo, 'i', val1, val1 + num - 1, nodeDelete1, nodeDelete2,list->line);
        list->line = list->line - num;

    } else {
        push(undo, 's', 0, 0, NULL, NULL,0);
    }
}

void print(struct sheetList* list, int val1, int val2){
    int i=0;

    if(val1==0){
        puts(".");
        val1++;
    }
    struct sheet* node = findNode(list,val1);
    for(i=val1; i<=val2 && i<=list->line && node!=list->nil; i++){
        printf("%s", node->text);
        node = node->next;
    }
    while(i<=val2){
        puts(".");
        i++;
    }

}

int main(int argc, char const *argv[]){

    char input_text[SIZE];
    char action;
    int val1=0, val2=0;
    int numberActionUndo, numberActionRedo;

    struct sheetList* list = (struct sheetList*) malloc(sizeof(struct sheetList));
    list->nil = (struct sheet*) malloc(sizeof(struct sheet));
    initializationTextlist(&list);

    struct stack* undo = (struct stack*) malloc(sizeof(struct stack));
    initializationStack(undo);
    struct stack* redo = (struct stack*) malloc(sizeof(struct stack));
    initializationStack(redo);

    while(fgets(input_text,SIZE,stdin) != NULL){

        if(strchr(input_text, ',')!=NULL){
            sscanf(input_text, "%d,%d%c", &val1,&val2,&action);
        } else if(strchr(input_text, 'q')!=NULL) {
            action = 'q';
        } else if (strchr(input_text, '.')==NULL){
            sscanf(input_text, "%d%c", &val1,&action);
        } else {
            action = '.';
        }

        if(action!='.'){

            switch (action){
                case 'c':{
                    deleteRedoList(redo);

                    create(list, undo, val1, val2);
                    break;
                }
                case 'p':{
                    //Stampa
                    print(list, val1, val2);
                    break;
                }
                case 'd':{
                    deleteRedoList(redo);

                    delete(list, undo, val1, val2);
                    break;
                }
                case 'u':{
                    int count = 0;
                    int numberMaxRedo = redo->top;
                    int numberMaxUndo = undo->top;
                    if(undo->top>0) {
                        while (action == 'u' || action == 'r') {
                            if (action == 'u') {
                                if(val1>numberMaxUndo){
                                    val1 = numberMaxUndo;
                                }
                                count += val1;
                                numberMaxUndo -= val1;
                                numberMaxRedo += val1;

                            } else if (action == 'r') {
                                if(val1>numberMaxRedo){
                                    val1=numberMaxRedo;
                                }
                                count -= val1;
                                numberMaxRedo -= val1;
                                numberMaxUndo += val1;
                            }

                            fgets(input_text, SIZE, stdin);
                            if (strchr(input_text, ',') != NULL) {
                                sscanf(input_text, "%d,%d%c", &val1, &val2, &action);
                            } else if (strchr(input_text, 'q') != NULL) {
                                action = 'q';
                                return 0;
                            } else if (strchr(input_text, '.') == NULL) {
                                sscanf(input_text, "%d%c", &val1, &action);
                            } else {
                                action = '.';
                            }
                        }

                        if (count > 0) {
                            executeUndo(list, undo, redo, count);
                        } else if (count < 0) {
                            executeUndo(list, redo, undo, (count * (-1)));
                        }

                        if (action == 'c') {
                            deleteRedoList(redo);

                            create(list, undo, val1, val2);
                        } else if (action == 'd') {
                            deleteRedoList(redo);

                            delete(list, undo, val1, val2);
                        } else if (action == 'p') {
                            print(list, val1, val2);
                        }
                    }
                    break;
                }
                case 'r':{
                    int count = 0;
                    int numberMaxRedo = redo->top;
                    int numberMaxUndo = undo->top;
                    if(redo->top>0){
                        while(action=='u' || action== 'r'){
                            if (action == 'u') {
                                if(val1>numberMaxUndo){
                                    val1 = numberMaxUndo;
                                }
                                count += val1;
                                numberMaxUndo -= val1;
                                numberMaxRedo += val1;

                            } else if (action == 'r') {
                                if(val1>numberMaxRedo){
                                    val1=numberMaxRedo;
                                }
                                count -= val1;
                                numberMaxRedo -= val1;
                                numberMaxUndo += val1;
                            }

                            fgets(input_text,SIZE,stdin);
                            if(strchr(input_text, ',')!=NULL){
                                sscanf(input_text, "%d,%d%c", &val1,&val2,&action);
                            } else if(strchr(input_text, 'q')!=NULL) {
                                action = 'q';
                                return 0;
                            } else if (strchr(input_text, '.')==NULL){
                                sscanf(input_text, "%d%c", &val1,&action);
                            } else {
                                action = '.';
                            }
                        }

                        if(count>0){
                            executeUndo(list,undo,redo,count);
                        } else if(count<0){
                            executeUndo(list,redo,undo,(count*(-1)));
                        }

                        if(action=='c'){
                            deleteRedoList(redo);

                            create(list,undo,val1,val2);
                        } else if(action=='d'){
                            deleteRedoList(redo);

                            delete(list,undo,val1,val2);
                        } else if(action=='p'){
                            print(list,val1,val2);
                        }
                    }

                    break;
                }
                case 'q':{
                    deleteRedoList(redo);
                    return 0;
                }
            }
        }
    }
}
