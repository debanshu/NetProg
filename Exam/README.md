Network Programming Lab Exam
============================

Solution to the Lab Exam.

####To Compile the program:

gcc NetProg_Server.c -lpthread

####To run

./a.out num_threads [port_number]

(port_number is optional, 5555 is default port used)

####Problems

UMSG <tname> <msg> seg-faults the server during the comparison (strcmp) call of <tname> with already recorded names.

####TODO:
- Add comments outlining the question.
