 Am implementat clientul in client.c. Am folosit:
 - flagul login_flag: indica daca user-ul este sau nu logat
 - flagul library_access: indica daca user-ul are sau nu acces la biblioteca
 - cookie_flag: pentru extragerea cookie-ului de sesiune
 - library_token: pentru extragerea token-ului 

 Dupa setarea adresei ip, programul intra intr-un loop infinit, in care citeste de la stdin una
dintre comenzile date si deschide o conexiune cu serverul.

 Comenzile:

 EXIT: - se inchide conexiunea si se iese din loop


 REGISTER: - se apeleaza functia 'getUserCredentials', care citeste de la stdin username-ul si
           parola si creeaza JSON-ul pentru request
           - se trimite JSON-ul la server folosind functia pentru POST request din request.c
           - se primeste raspunsul
           - se afiseaza mesajul corespunzator de succes sau de eroare.


 LOGIN:  - se verifica daca user-ul este logat
         - se creeaza obiectul JSON (folosind 'getUserCredentials')
         - se trimite JSON-ul la server folosind functia pentru POST request din request.c
         - se primeste raspunsul
         - daca logarea a reusit, se extrage cookie-ul de sesiune si flagul de logare se seteaza pe 1
         - altfel, se afiseaza mesajul de eroare.


 ENTER_LIBRARY: - folosesc functia 'check_flags' pentru a verifica daca user-ul este logat si
                daca are deja acces la biblioteca
                - se trimite un request de tip GET cu cookie-ul de sesiune setat
                - se extrage token-ul
                - flagul de acces se seteaza pe 1 si se afiseaza mesajul de succes.


 GET_BOOKS:  - se verifica daca user-ul are acces la biblioetca
             - daca are, se trimite un request de tip GET cu cookie-ul si token-ul setate
             - se extrage lista cartilor in format JSON si se afiseaza in formatul corespunzator
             - daca nu exista nicio carte in biblioteca, se afiseaza un mesaj de eroare.


GET_BOOK:  - se verifica daca user-ul are acces la biblioteca
           - daca are, se apeleaza functia 'get_url', care citeste de la stdin id-ul cartii si alcatuieste URL-ul pentru request
           - se trimite un request de tip GET cu cookie-ul si token-ul setate
           - se extrage cartea in format JSON si se afiseaza
           - daca cartea cu id-ul respectiv nu exista, se afiseaza un mesaj de eroare.


 ADD_BOOK:  - se verifica daca user-ul are acces la biblioteca
            - se apeleaza functia 'get_book', care citeste de la stdin titlul, autorul, 
            publisher-ul si genul cartii si creeaza JSON-ul
            - se trimite un request de tip POST cu cookie-ul si token-ul setate
            - se afiseaza mesajul corespunzator de succes sau de eroare.


 DELETE_BOOK:  - se verifica daca user-ul are acces la biblioteca
               - daca are, se apeleaza functia 'get_url' pentru a face rost de 
               URL pentru request
               - se trimite un request de tip DELETE
               - se afiseaza mesajul corespunzator de succes sau de eroare.


 LOGOUT:  - se verifica daca user-ul este logat
          - daca da, se afiseaza mesajul de succes si se reseteaza flagurile, cookie-ul si token-ul.

 
 Pentru implementarea proiectului, am pornit de la scheletul din laborator si am adaugat 
biblioteca 'parson' de parsare JSON, asa cum a fost recomandat in cerinta. Am ales biblioteca 'parson' pentru manipularea JSON datorita simplitatii si usurintei de utilizare, dimensiunii reduse, performantei bune si documentatiei clare, care contribuie la un cod curat si eficient.

 Pentru verificarea formatului input-ului de la user, am folosit functia 'string_check', care
verifica daca input-ul este gol sau daca acesta contine spatii. Am folosit functia 'extract_id' pentru a extrage id-ul cartii din URL.
