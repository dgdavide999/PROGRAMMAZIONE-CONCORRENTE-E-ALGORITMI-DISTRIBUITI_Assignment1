<i>data progetto: 15 aprile 2022</i>

# PCAD 2021-22, Foglio 1: Architetture e parallelismo

Un modo molto semplice di parallelizzare calcoli su matrici è basato sulla decomposizioni in
blocchi delle matrici sulle quali si opera.<br>
Ad es. consideriamo una matrice A (M x N) e un vettore V (N x 1).<br>
Nella decomposizione per righe la matrice A si divide in T blocchi di R righe (M=TxR)<br>
Ogni blocco RxN si può quindi moltiplicare per V ottenendo un nuovo vettore R x 1<br>
Concatenando i T vettori risultanti (vedi figura sotto) otteniamo il vettore A x V (M x 1).<br>

<img src="./parallelizzazione_es.png" alt="immagine matrice divisa per thread"><br>
Nel caso di prodotto di matrice per matrice l’applicazione della decomposizione produce R
righe della matrice risultante.<br>
Lo scopo dell’esercizio è quello di scrivere un programma C con la libreria pthread per
applicare la decomposizione per righe delegando la moltiplicazione blocco per blocco a
diversi thread.<br>
Più precisamente il programma deve effettuare la moltiplicazione C * (A * B) tra matrici di
float dove:
<ul>
<li>A è una matrice MxN,
<li>B è una matrice NxP,
<li>C è una matrice PxM
</ul>

Per sfruttare il multithreading utilizzare la decomposizione per righe per calcolare sia R=A*B
che C*R (il risultato è una matrice PxP).<br>
<br>
Utilizzate gli stessi thread nelle due fasi di calcolo applicando una barriera per sincronizzare
le due fasi di calcolo: i thread devono attendere in barriera il completamento della prima
parte di calcolo e dopo la seconda decomposizione completare il calcolo.<br>
Fate in modo che sia possibile configurare il numero di blocchi delle due decomposizioni in
maniera da misurare l’eventuale aumento di performance al variare del numero di thread.<br>
Per analizzare i tempi di esecuzione potete usare le funzioni della libreria “time.h”.


### Risultato
<img src="./output.png" alt="immagine matrice divisa per thread"><br>
