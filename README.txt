Voicu Catalin Marian
322CC

Tema am abordat-o incepand cu laboratorul 7.

Abonatii sunt reprezentati prin clasa Client, in care se regaseste ID-ul
clientului, socketul de comunicare, starea de conectare, storage pentru
starea forward, topicul, si mesajele.

Topic-ul se caracterizeaza prin nume si un vector de clienti. De asemenea aici
putem vedea daca un client este abonat sau nu la topic, daca este putem
verifica daca are sf activat.

Home-ul este o clasa ce retine aspectele generale ale aplicatiei.

Clientul si serverul folosesc multiplexarea I/O pentru a primi mesaje de la mai
multe surse(implementare bazata pe implementarea din laborator)

In ceea ce priveste interpretarea mesajelor ce vin de la clientii udp, am
definit structurile care ajuta la interpretarea mesajelor.

Pentru server, singura comanda acceptata de la tastatura este exit, care
inchIDe programul, dar si conexiunile cu clientii conectati la server.

Cand un client tcp se logheaza, acesta trimite la server un mesaj cu ID-ul sau.
In momentul in care clientul se conecteza, se verifica daca acesta a
mai fost conectat si daca are mesaje de la clientii udp care au fost trimise
cand acesta nu era conectat, astfel serverul i le trimite.

Clientii tcp pot primi comenzile subscribe, unsubscribe si exit. Pentru
subscribe si unsubscribe clientul trimite la server un mesaj cu tipul comenzii,
topicul, si eventual sf ul in cazul subscribe.De asemenea este de precizat
ca atunci cand conexiunea este refuzata unui client, sau inchisa, se trimite la
client mesajul "Already connected", deoarece comportamentul este similar, am
refolosit acest mesaj.

Tratarea erorilor: la server singura comanda acceptata de la tastatura este
exit. Pentru clientii tcp, se verifica daca s-a introdus o comanda de exit subscribe
sau unsubscribe. Subscribe asteapta 2 argumente, numele topicului, iar pentru 
activarea sf ului se introduce 1. In cazul in care un apel de sistem esueaza, am
folosit utilitarul din laborator DIE, se afiseaza un mesaj la stderr si
programul se inchide. 