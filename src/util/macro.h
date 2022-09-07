#pragma once
#define CONCAT_(A, B) A ## B
#define CONCAT(A, B) CONCAT_(A, B)

#define EVAL(M) M