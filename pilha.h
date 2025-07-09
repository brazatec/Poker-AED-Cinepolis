#ifndef PILHA_H
#define PILHA_H
#include <stdio.h>
#define MAX 100
typedef struct{
    char valor[3];
    char naipe[11];
}cartas;

typedef struct{
    int topo;
    cartas item[MAX];
} tp_pilha;

void inicializaPilha(tp_pilha *p){
    p->topo = -1;
}

int pilhaVazia(tp_pilha *p){
    if (p->topo == -1) return 1;
    return 0;
}

int pilhaCheia(tp_pilha *p){
    if (p->topo == MAX - 1){
        return 1;
    }
    else{
        return 0;
    }
}

int push(tp_pilha *p , cartas e){
    if (pilhaCheia(p)==1) return 0;
    p->topo++;
    p->item[p->topo]=e;
    return 1;
}

int pop(tp_pilha *p , cartas *e){
    if (pilhaVazia(p) == 1) return 0;
    *e=p->item[p->topo];
    p->topo--;
    return 1;
}

int top(tp_pilha *p , cartas *e){
    if (pilhaVazia(p) == 1) return 0;
    *e=p->item[p->topo];
    return 1;
}

int alturaPilha(tp_pilha *p){
    return p->topo + 1;
}

void imprimePilha(tp_pilha p){
    cartas e;
    
    while (!pilhaVazia(&p)){
        pop(&p , &e);
        printf("%s %s \n" , e.valor , e.naipe);
    }
}

#endif