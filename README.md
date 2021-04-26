Nume: Badita Rares Octavian
Grupa: 333CB

# Tema 3 Sisteme de Operare

Organizare
-

In faza de intializarea a loaderului, salvez handlerul initial pentru SIGESGV pentru a-l utiliza mai tarziu si inlocuiesc cu ```void so_sigaction(int, siginfo_t*, void*)``` pentru tratarea page fault-urilor.

Scopul functiei ```so_sigaction``` este de a gasi segmentul in care s-a produs semnalul de SIGSEGV. Daca adresa la care s-a produs semnalul nu se regaseste in vreun segment, atunci se apleaza handlerul original.

Odata gasit segmentul se apleaza functia ```void so_map_page(uintptr_t, so_seg_t*)``` ce prineste adresa page fault-ului si referinta catre structura de segment. Procesul de mapare decurge astfel:
	1. Daca vectorul de data (unde se salveaza daca paginile au fost mapate) nu este initializat, atunci se intializeaza;
	1. Se determina pagina si adresa ei de start. Daca pagina a fost mapata deja, inseaman ca a existat o eroare de permisiuni;
	1. Se mapeaza pagina si se actualizeaza vectorul data din segment;
	1. Daca adresa realativa a paginii se afla inainte de ```file_size```, atunci se copiaza din fisierul sursa la adresa mapata;
	1. In final se stabilesc protectiile pentru pagina.

Implementare
-

* Tema este realizata integral, inclusiv testul de coding style.
* Nu este implementata functionalitatea de Windows a temei.


Resurse Utile
-

* Au fost foarte utile paginile de manual de linux ale functiilor [mman](https://man7.org/linux/man-pages/man2/mmap.2.html) si [mprotect](https://man7.org/linux/man-pages/man2/mprotect.2.html).
* [Link](https://stackoverflow.com/questions/6015498/executing-default-signal-handler) cu metoda sa salvezi si sa reutilizez handlerul original al semnalului.

[Github](https://github.com/WhyNotRaresh/Tema3SO)
-
