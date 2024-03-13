#include <stdio.h>
#include <stdlib.h>
// define per hash func
#define DimTab 233
#define Tomb (void *)"TOMBA"

// STRUCT-------------------------------------------------------------------------------------------------------------------------------------
typedef struct BSTAuto
{
    struct BSTAuto *Left;  // Figlio sinistro
    struct BSTAuto *Right; // Figlio destro
    long int Aut;          // Valore auto
} BSTAuto;
typedef struct Perco // Lista percorso finale
{
    struct Perco *Next;
    long int Dist;
} Perco;
typedef struct Stazione
{
    long int SDist;   // Distanza (data da ut)
    BSTAuto *AlbA;    // Elenco Auto nella stazione
    long int MaxAuto; // Autonomia dell'auto piu lunga
    int Pool;         // Numero di auto all' interno del bst

} Stazione;
typedef struct LStat
{
    struct LStat *Next;
    Stazione *Station;
} LStat;
// Globali------------------------------------------------------------------------------------------------------------------------------------
Stazione **TAB;   // HASH TABLE DEI PUNTATORI ALLE STAZIONI (Se supera 2/3 si rialloca)
int DT = DimTab;  // Dimensione tabella (dipende da riallocazioni)
int Presenze = 0; // Conta quanti elementi ci sono all' interno della tabella
int NTomb = 0;    // Numero di Tombstone
// Per funzione CPN
Perco *PossibilePercorso = NULL; // Testa percorso
// Er = scanf (Necessaria per compilazione)
int Er;
// FUNZIONI-----------------------------------------------------------------------------------------------------------------------------------
int Hash(long int, int);                                    // Funzione di HASH variabile(DT)
int IIT(Stazione *);                                        // Inserisci In Tab
int SIT(long int);                                          // Search In Tab ed eventualmente elimina
int AST();                                                  // Inserisci Stazione
int DST();                                                  // Rimuovi stazione
void PP();                                                  // Pianifica percorso
int RAuto();                                                // Rottama Auto
void DeallocaAuto(BSTAuto *);                               // Dealloca l'albero delle Auto di una determinata Stazione
void ReHash(int);                                           // Ricrea la hash table ma il doppio piu grande
void InsAuto(BSTAuto *, long int);                          // Inserisce Auto nell'albero
int AggAuto();                                              // Aggiunge auto a stazione gia esistente
int ElAuto(BSTAuto *, long int, int);                       // Elimina l'auto selezionata;
void StPercorso(Perco *);                                   // Stampa la lista del percorso trovato
int CPN(Stazione **, Stazione *, int, int);                 // Calcola Percorso da Sx a Dx
void CPP(Stazione **, long int, int, Perco *);              // Calcola Percorso da Dx a SX
void Calcolapercorso(Stazione **, long int, long int, int); // Imposta i parametri per le 2 funzioni precedenti
int FunQuickSort(const void *, const void *);               // Funzione necessaria al qsort
void EliminaPercorso(Perco *);                              // Elimina la lista passata come parametro

void StampHash();       // Debug Function
void Stampa(BSTAuto *); // Debug Function

// main --------------------------------------------------------------------------------------------------------------------------------------

int main()
{
    char c;
    TAB = calloc(DT, sizeof(Stazione *));
    do
    {
        c = getchar();
        switch (c) // COMANDI
        {

        case 'a': // AGGIUNGI
        {
            Er = scanf("%*[^-]%*c");
            switch (getchar())
            {
            case 'a': // Auto  OK
            {
                if (AggAuto())
                    printf("aggiunta\n");
                else
                    printf("non aggiunta\n");
            }
            break;
            case 's': // Stazione OK
            {
                if (AST())
                    printf("aggiunta\n");
                else
                    printf("non aggiunta\n");
            }
            break;

            default:
                break;
            }
        }
        break;
        case 'd': // Demolisci-Stazione OK
        {

            if (DST())
                printf("demolita\n");
            else
                printf("non demolita\n");
        }
        break;
        case 'r': // Rottama-Auto
        {
            if (RAuto())
                printf("rottamata\n");
            else
                printf("non rottamata\n");
        }
        break;
        case 'p': // Pianifica-Percorso
        {
            PP();
        }
        break;
        default:
            break;
        }

    } while (c != EOF);
    // StampHash();
    return 0;
}

// FUNZIONI-----------------------------------------------------------------------------------------------------------------------------------
int Hash(long int Key, int i) // Funzione di HASH variabile(DT)
{
    int Pos;
    // Pos = ((Key * 11) % (DT * 3) + i * (2 * (Key * 7) % (DT * 5) + 1)) % DT;
    Pos = ((Key % DT) + i * (1 + (Key % DT) % (DT - 1))) % DT; // Funzione di Hash con meno collisioni
    return Pos;
}
int IIT(Stazione *Punt) // Inserisci In Tab
{
    static int Riallocazioni = 0; // Speedy way per non passarsela come parametro
    int Pos, Stat = Punt->SDist;  // Indice Tabella Hash, Valore Stazione
    int i = 0, T = 0;
    Stazione *Temp;
    do
    {
        Pos = Hash(Stat, i);
        i++;
        Temp = TAB[Pos];
        if (Temp)
        {
            if (Temp == Tomb)
                T = 1;
            else if (Temp->SDist == Stat) // collisione di elemento gia presente (fine procedura)
                return 0;
        }
    } while (Temp && i != DT && !T); // Ciclo per trovare il posto alla nuova stazione inserita
    if (!Temp)
    {
        TAB[Pos] = Punt; // Inserimento in Hash table
        Presenze++;
        if (Presenze > (DT / 4) * 3) // Se il fattore di carico è maggiore del 75% Rialloco max 9 volte (max 150000 elementi ~)
        {
            if (Riallocazioni < 9)
            {

                ReHash(Riallocazioni); // Riallocazione della Tabella
                Riallocazioni++;
            }
            else
                return 0;
        }
        return 1;
    }
    if (T) // Rimpiazzo di una tomba con il puntatore nuovo
    {
        TAB[Pos] = Punt;
        NTomb--;
        return 1;
    }
    return 0;
}
int SIT(long int Key) // Search In Tab
{
    int Pos, i;
    Stazione *Punt;
    for (i = 0; i < DT; i++)
    {
        Pos = Hash(Key, i);
        Punt = TAB[Pos];
        if (Punt == NULL)
            return -1;
        if (Punt->SDist == Key)
            return Pos;
    }
    return -1;
}
int AST() // Aggiungi Stazione
{
    int i, PoolD;
    long int Dist, ValA, MaxA = 0;
    BSTAuto *Root; // punt auto;
    Stazione *Station;
    Er = scanf("%*[^ ]%*c");  // elimino il restante s-"tazione" sul buffer
    Er = scanf("%ld", &Dist); // prendo la distanza (key)
    Er = scanf("%d", &PoolD); // Prendo il num di elementi della lista che sarà poi un bst
    if (PoolD > 512 || SIT(Dist) != -1 || PoolD < 0)
    {
        Er = scanf("%*[^\n]%*c"); // Elimino tutto il resto del comando fino al successivo
        return 0;
    }
    Root = malloc(sizeof(BSTAuto));
    Root->Left = NULL;
    Root->Right = NULL;
    if (PoolD) // se la stazione ha piu di 0 macchine
    {
        Er = scanf("%ld", &ValA);
        Root->Aut = ValA;
        MaxA = ValA;
        for (i = 0; i < PoolD - 1; i++)
        {
            Er = scanf("%ld", &ValA);
            if (ValA > MaxA)
                MaxA = ValA; // Tengo il valore massimo per il conteggio percorsi piu avanti!
            InsAuto(Root, ValA);
        }
    }

    Station = malloc(sizeof(Stazione)); // allocazione per inserire in tab
    Station->SDist = Dist;
    Station->MaxAuto = MaxA;
    Station->AlbA = Root;
    Station->Pool = PoolD;
    getchar(); // prende l'invio finale
    if (IIT(Station))
        return 1;
    return 0;
}
int DST() // Demolisci stazione
{
    int Pos;
    long int Dist;

    Stazione *Punt;

    Er = scanf("%*[^ ]%*c"); // elimino il restante sul buffer
    Er = scanf("%ld", &Dist);
    getchar(); // Sempre l'invio da levare
    Pos = SIT(Dist);

    if (Pos == -1)
        return 0;
    Punt = TAB[Pos];
    DeallocaAuto(Punt->AlbA); // svuoto la pool di auto
    Punt->AlbA = NULL;
    Punt->SDist = -1;
    Punt->Pool = 0;
    TAB[Pos] = Tomb;
    NTomb++;
    return 1;
}
int RAuto() // Rottama Auto
{
    long int Dist, Val;
    int Pos;
    Er = scanf("%*[^ ]%*c"); // Svuoto il buffer
    Er = scanf("%ld", &Dist);
    Er = scanf("%ld", &Val);
    getchar();

    Pos = SIT(Dist);
    if (Pos == -1)
        return 0;
    if (ElAuto(TAB[Pos]->AlbA, Val, Pos)) // Elimino la macchina dal bst
    {
        TAB[Pos]->Pool--; // Rimuovo 1 macchina dalla pool
        return 1;
    }
    return 0;
}
void DeallocaAuto(BSTAuto *Alb) // Dealloca l'albero delle Auto di una determinata Stazione
{
    if (Alb->Left)
        DeallocaAuto(Alb->Left);
    if (Alb->Right)
        DeallocaAuto(Alb->Right);

    free(Alb);
    return;
}
void ReHash(int Redo) // Ricrea la hash table ma il doppio piu grande
{
    int Primi[10] = {571, 1009, 2789, 4759, 8933, 19001, 28393, 48029, 99809, 153641}; // Primi che sono circa il doppio l'uno dell' altro per dimensioni Hash
    int VDT = DT, i;
    DT = Primi[Redo];
    Stazione **TTAB = TAB; // Vecchia tabella (piu piccola)

    TAB = calloc(DT, sizeof(Stazione *));
    Presenze = 0;
    NTomb = 0;
    for (i = 0; i < VDT; i++) // Ricopiatura Tabella nuova e eliminazaione della vecchia
        if (TTAB[i] && TTAB[i] != Tomb)
            IIT(TTAB[i]);

    free(TTAB);
    return;
}
void InsAuto(BSTAuto *Nod, long int val) // Inserisce Auto nell'albero
{
    BSTAuto *Nuova;
    if (val < Nod->Aut) // Minore
    {
        if (Nod->Left)
            InsAuto(Nod->Left, val);
        else
        {
            Nuova = malloc(sizeof(BSTAuto));
            Nuova->Left = NULL;
            Nuova->Right = NULL;
            Nuova->Aut = val;

            Nod->Left = Nuova;
        }
    }
    else // Maggiore/Uguale
    {
        if (Nod->Right)
            InsAuto(Nod->Right, val);
        else
        {
            Nuova = malloc(sizeof(BSTAuto));
            Nuova->Left = NULL;
            Nuova->Right = NULL;
            Nuova->Aut = val;

            Nod->Right = Nuova;
        }
    }
    return;
}
int AggAuto() // Aggiunge auto a stazione gia esistente
{
    long int Dist, Val;
    int Pos;
    Stazione *Punt;
    Er = scanf("%*[^ ]%*c"); // elimino il restante a-"uto " sul buffer
    Er = scanf("%ld", &Dist);
    Er = scanf("%ld", &Val);
    getchar();
    Pos = SIT(Dist);
    if (Pos == -1)
        return 0;
    Punt = TAB[Pos];
    if (Punt->Pool + 1 > 512) // Se per caso si aggiunge un auto in più rispetto alla specifica annullo
        return 0;
    InsAuto(Punt->AlbA, Val);
    if (Val > Punt->MaxAuto)
        Punt->MaxAuto = Val;
    Punt->Pool++;
    return 1;
}
int ElAuto(BSTAuto *Nod, long int val, int Pos) // Elimina l'auto selezionata
{

    BSTAuto *p, *Figlio, *FiglioD, *FiglioS;
    Stazione *Punt;
    if (Nod != NULL)
    {

        if (val > Nod->Aut)
        {

            Punt = TAB[Pos];
            Figlio = Nod->Right;

            if (Figlio == NULL)
                return 0;

            if (Figlio->Aut != val)
                return ElAuto(Figlio, val, Pos);
            else // Rimuovi destra
            {
                if (val == Punt->MaxAuto)
                {
                    if (Figlio->Left)
                    {
                        p = Figlio->Left;
                        while (p->Right != NULL)
                            p = p->Right;
                        Punt->MaxAuto = p->Aut;
                    }
                    else
                        Punt->MaxAuto = Nod->Aut;
                }

                if (Figlio->Left == NULL && Figlio->Right == NULL)
                {
                    Nod->Right = NULL;
                    free(Figlio);
                }
                else if (Figlio->Right != NULL && Figlio->Left == NULL)
                {
                    p = Figlio->Right;
                    free(Figlio);
                    Nod->Right = p;
                }
                else if (Figlio->Left != NULL && Figlio->Right == NULL)
                {
                    p = Figlio->Left;
                    free(Figlio);
                    Nod->Right = p;
                }
                else
                {
                    p = Figlio->Right;

                    while (p->Left != NULL)
                        p = p->Left;
                    p->Left = Figlio->Left;

                    p = Figlio->Right;
                    free(Figlio);
                    Nod->Right = p;
                }
                return 1;
            }
        } //------------------------------------------------------------
        else if (val < Nod->Aut)
        {

            Figlio = Nod->Left;

            if (Figlio == NULL)
                return 0;
            if (Figlio->Aut != val)
                return ElAuto(Figlio, val, Pos);
            else // Rimuovi sinistra
            {
                if (Figlio->Left == NULL && Figlio->Right == NULL)
                {
                    Nod->Left = NULL;
                    free(Figlio);
                }
                else if (Figlio->Right != NULL && Figlio->Left == NULL)
                {
                    p = Figlio->Right;
                    free(Figlio);
                    Nod->Left = p;
                }
                else if (Figlio->Left != NULL && Figlio->Right == NULL)
                {
                    p = Figlio->Left;
                    free(Figlio);
                    Nod->Left = p;
                }
                else
                {
                    p = Figlio->Right;

                    while (p->Left != NULL)
                        p = p->Left;
                    p->Left = Figlio->Left;

                    p = Figlio->Right;
                    free(Figlio);
                    Nod->Left = p;
                }
                return 1;
            }
        }
        else
        {
            FiglioD = Nod->Right;
            FiglioS = Nod->Left;
            Punt = TAB[Pos];

            if (val == Punt->MaxAuto)
            {
                if (FiglioS)
                {
                    p = FiglioS;
                    while (p->Right != NULL)
                        p = p->Right;
                    Punt->MaxAuto = p->Aut;
                }
                else
                    Punt->MaxAuto = 0;
            }

            if (FiglioD == NULL && FiglioS == NULL)
            {

                TAB[Pos]->MaxAuto = 0;
                free(Nod);
                Nod = NULL;
            }

            else if (FiglioD != NULL && FiglioS == NULL)
            {
                p = FiglioD;
                free(Nod);
                Nod = p;
            }
            else if (FiglioS != NULL && FiglioD == NULL)
            {
                p = FiglioS;
                free(Nod);
                Nod = p;
            }
            else
            {
                p = FiglioD;

                while (p->Left != NULL)
                    p = p->Left;
                p->Left = FiglioS;

                p = FiglioD;
                free(Nod);
                Nod = p;
            }
            TAB[Pos]->AlbA = Nod;

            return 1;
        }
    }
    return 0;
}
void PP() // Pianifica percorso
{
    int i, j = 1;
    long int S1, S2, N1, N2;
    Stazione *Casella;
    Stazione **V;
    LStat *Lista = NULL, *Temp = NULL, *Temp2 = NULL;
    Er = scanf("%*[^ ]%*c");
    Er = scanf("%ld", &S1);
    Er = scanf("%ld", &S2);
    getchar();
    if (S1 == S2)
    {
        printf("%ld\n", S1);
        return;
    }
    N1 = S1;
    N2 = S2;
    if (S1 > S2)
    {
        N1 = S2;
        N2 = S1;
    }
    Lista = malloc(sizeof(LStat));
    Lista->Station = TAB[SIT(N1)];
    Temp = Lista;
    for (i = 0; i < DT; i++)
    {
        Casella = TAB[i];
        if (Casella && Casella != Tomb && Casella->SDist <= N2 && Casella->SDist > N1)
        {
            Temp2 = malloc(sizeof(LStat));
            Temp->Next = Temp2;
            Temp = Temp->Next;
            Temp2->Station = Casella;
            Temp2->Next = NULL;
            j++;
        }
    }

    V = malloc(sizeof(Stazione *) * j);
    for (i = 0; i < j && Lista != NULL; i++)
    {
        V[i] = Lista->Station;
        Lista = Lista->Next;
    }
    Temp = Lista;
    while (Temp != NULL)
    {
        Temp2 = Temp->Next;
        free(Temp);
        Temp = Temp2;
    }
    qsort(V, j, sizeof(Stazione *), FunQuickSort);
    // V ordinato
    Calcolapercorso(V, S1, S2, j - 1);

    free(V);
    return;
}
int FunQuickSort(const void *a, const void *b) // funzione necessaria al qsort
{
    return (*((Stazione **)a))->SDist - (*((Stazione **)b))->SDist;
}
void Calcolapercorso(Stazione **V, long int a, long int b, int j) // Imposta i parametri per le 2 funzioni precedenti
{
    Perco *Perc;
    Perc = malloc(sizeof(Perco));
    Perc->Dist = V[j]->SDist;
    Perc->Next = NULL;
    if (a < b)
    {
        CPP(V, V[j]->SDist, j, Perc);
    }
    else
    {

        if (CPN(V, V[j], j, j - 1) < 0)
            printf("nessun percorso\n");
        else
        {
            printf("%ld ", V[j]->SDist);
            StPercorso(PossibilePercorso);
            EliminaPercorso(PossibilePercorso);
            PossibilePercorso = NULL;
        }
    }
    return;
}
void CPP(Stazione **V, long int a, int j, Perco *H) // Calcola Percorso da Sx a Dx
{
    int i, maxlontano = j, Trovato = 0;
    Perco *temp;
    Stazione *Stat;
    for (i = 0; i < j && Trovato == 0; i++)
    {
        Stat = V[i];
        if (Stat->MaxAuto + Stat->SDist >= a)
        {
            Trovato = 1;
            maxlontano = i;
        }
    }

    if (maxlontano == j)
    {
        printf("nessun percorso\n");

        for (temp = H; temp != NULL; H = temp)
        {
            temp = H->Next;
            free(H);
        }
        return;
    }
    else
    {
        temp = H;
        H = malloc(sizeof(Perco));
        H->Next = temp;
        H->Dist = V[maxlontano]->SDist;
    }

    if (H->Dist == V[0]->SDist)
    {
        StPercorso(H);
        EliminaPercorso(H);
        return;
    }
    else
        CPP(V, V[maxlontano]->SDist, maxlontano, H);
    return;
}
int CPN(Stazione **V, Stazione *StazCorrente, int InSt, int Ragg) // Calcola Percorso da Dx a SX
{
    int i, j, InMin = InSt, Esito, Trovato = 0;
    long int DiffRagg = StazCorrente->SDist - StazCorrente->MaxAuto, Min = DiffRagg, Diff;
    Stazione *StazContr = V[Ragg];
    Perco *OldStation;

    for (i = Ragg; i >= 0 && StazContr->SDist >= DiffRagg;) // for che permette di trovare la stazione che può andare più lontano
    {                                                       // e la stazione a cui è possibile arrivare dalla stazione passata come parametro
        Diff = StazContr->SDist - StazContr->MaxAuto;
        if (Diff <= Min || Diff <= 0)
        {
            Min = Diff;
            InMin = i; // InMin è l'indice della stazione col "raggio d'azione" più lungo raggiungibile e i (alla fine) sarà l'indice dell' ultima stazione raggiungibile dalla stazione chiamante
        }
        i--;
        if (i >= 0)
            StazContr = V[i];
    }
    i++;
    if (i == 0) // Arrivati alla fine del vettore esiste almeno un percorso
    {
        PossibilePercorso = malloc(sizeof(Perco));
        PossibilePercorso->Dist = V[0]->SDist;
        PossibilePercorso->Next = NULL;
        return 0; // Ho allocato la stazione di destinazione e tramite inserimenti in testa costruisco il percorso definitivo
    }

    if (InSt == InMin) // La stazione chiamante può raggiungere solo se stessa (fine della funzione)
        return -1;

    Esito = CPN(V, V[InMin], InMin, i - 1); // Richiamo la funzione sulla stazione minima. Ottimizzando anche il fatto che le stazioni nel raggio d'azione del chiamante sono gia state viste e quindi inutile riguardarle
    if (Esito == -1)                        //-1 viene restituito se non c'è alcun percorso (uno dei minimi non raggiunge la destinazione), inutile continuare la funzione continuerà a ritornare -1
        return -1;
    for (j = i; j < InMin && !Trovato; j++) // j diventa la stazione più a sinistra (priorità massima) E scorre fino a che la stazione non raggiunge la minima oppure la prima stazione che raggiunge la minima successiva
    {
        if (V[j]->SDist - V[j]->MaxAuto <= V[Esito]->SDist)
        {
            Trovato = 1; // Finita il ciclo se ne trova
            j--;         // sarebbe incrementato dal for
        }
    }
    OldStation = PossibilePercorso; // alloco la stazione più a sinistra possibile che raggiunge le altre
    PossibilePercorso = malloc(sizeof(Perco));
    PossibilePercorso->Dist = V[j]->SDist;
    PossibilePercorso->Next = OldStation;
    return j;
}
void StPercorso(Perco *H) // Stampa la lista del percorso trovato
{
    Perco *Temp;
    for (Temp = H; Temp->Next != NULL; Temp = Temp->Next)
        printf("%ld ", Temp->Dist);
    printf("%ld\n", Temp->Dist);
    return;
}
void EliminaPercorso(Perco *PP) //Elimina i percorsi temporanei generati dalle funzioni CPP e CPN
{
    Perco *Temp;

    while (PP != NULL)
    {
        Temp = PP->Next;
        free(PP);
        PP = Temp;
    }
    return;
}
void Stampa(BSTAuto *Head)
{
    if (Head->Left != NULL)
    {
        Stampa(Head->Left);
    }
    printf("%d |", Head->Aut);
    if (Head->Right != NULL)
    {
        Stampa(Head->Right);
    }

    return;
}
void StampHash() //funzione di debug. Stampa l'intera tabella Hash delle stazioni.
{
    int i;
    Stazione *Punt;
    printf("\n");
    if (DT)
        printf("%d ", DT);
    else
    {
        for (i = 0; i < DT; i++)
        {
            if (TAB[i])
            {
                if (TAB[i] != Tomb)
                {
                    printf("\n%d:\t\t%d\t", i + 1, TAB[i]->SDist);
                    Stampa(TAB[i]->AlbA);
                    printf("   (%d)   [%d]", TAB[i]->MaxAuto, TAB[i]->Pool);
                    Punt = TAB[i];
                    DeallocaAuto(Punt->AlbA);
                    Punt->AlbA = NULL;
                    Punt->SDist = -1;
                }
                else
                    printf("\n%d:\t\t%c", i + 1, 'T');
            }

            else
                printf("\n%d:\t\t-----", i + 1);
        }
        free(TAB);
        printf("\n\nPresenze: %d\n\n", Presenze);
        getchar();
        getchar();
    }

    return;
}
