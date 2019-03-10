#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphviz/cgraph.h>
#include "grafo.h"

typedef struct nodo_lista *nodo_lista_p;
typedef struct lista *lista_p;
typedef struct aresta *aresta_p;

#pragma pack (1)
struct nodo_lista {
  void *conteudo;
  nodo_lista_p proximo;
} nodo_lista_t;

struct lista {
  unsigned int tam;
  nodo_lista_p primeiro; 
} lista_t;

struct aresta {
  vertice origem, destino;
} aresta_t;

//------------------------------------------------------------------------------
// (apontador para) estrutura de dados para representar um grafo
// 
// o grafo pode ser direcionado ou não
// 
// o grafo tem um nome, que é uma "string"
struct grafo {
  char *nome;
  lista_p vertices, arestas;
};

struct vertice {
  char *nome;
  unsigned int cor;
  unsigned int rotulo;
  unsigned int visitado;
  lista_p entrada, saida;
};

//------------------------------------------------------------------------------
// Funcao de erro padrao
//
// Recebe como entrada uma mensagem de erro
//
// Gera como saida uma mensagem de erro
inline static void err_exit(const char *msg) {
  printf("[Fatal Error]: %s \nExiting... \n", msg);
  exit(1);
}

//------------------------------------------------------------------------------
// Aloca e devolve uma lista de apontadores
//
// Devolve a lista em caso de sucesso, ou NULL, caso contrário
static lista_p cria_lista(void) {
  lista_p lista = (lista_p)malloc(sizeof(lista_t));
  if(!lista)
    err_exit("Erro ao criar lista!");

  lista->primeiro = NULL;
  lista->tam = 0;

  return lista;
}

//------------------------------------------------------------------------------
// Desaloca uma lista de apontadores
//
// Recebe como entradada uma lista de apontadores
//
// Devolve 1 em caso de sucesso, ou 0 caso, contrário
static int destroi_lista(lista_p lista) {
  if(!lista)
    return 0;

  nodo_lista_p nodo;

  while(lista->primeiro) {
    nodo = lista->primeiro;
    lista->primeiro = lista->primeiro->proximo;

    free(nodo->conteudo);
    free(nodo);
  }

  free(lista);

  return 1;
}

//------------------------------------------------------------------------------
// Desaloca uma lista de apontadores
//
// Recebe como entradada uma lista de apontadores
//
// Devolve a lista em caso de sucesso, ou NULL, caso contrário
static nodo_lista_p cria_nodo_lista(void *conteudo) {
  nodo_lista_p nodo = (nodo_lista_p)malloc(sizeof(nodo_lista_t));
  if(!nodo)
    err_exit("Erro ao criar nodo!");

  nodo->conteudo = conteudo;
  nodo->proximo = NULL;

  return nodo;
}

//------------------------------------------------------------------------------
// Adiciona um nodo em uma lista de apontadores
//
// Recebe como entradada uma lista de apontadores e um nodo
static void add_nodo_lista(lista_p lista, nodo_lista_p nodo) {
  nodo_lista_p p = lista->primeiro;

  if(p != NULL) {

    while(p->proximo != NULL) {
      p = p->proximo;
    }

    p->proximo = nodo;
    nodo->proximo = NULL;

  } else {
    lista->primeiro = nodo;
  }

  lista->tam++;
}

//------------------------------------------------------------------------------
// Aloca a estrutura de um grafo
//
// Devolve um grafo em caso de sucesso, ou NULL, caso contrário
static grafo cria_grafo(void) {
  grafo g = (grafo)malloc(sizeof(struct grafo));
  if(!g)
    err_exit("Erro ao criar novo grafo!");

  g->nome = (char *)malloc(256 * sizeof(char));
  if(!g->nome)
    err_exit("Erro ao criar grafo->nome!");

  g->vertices = cria_lista();
  g->arestas = cria_lista();

  return g;
}

//------------------------------------------------------------------------------
// desaloca toda a memória usada em *g
// 
// devolve 1 em caso de sucesso,
//         ou 
//         0, caso contrário
int destroi_grafo(grafo g) {
  if(!g)
    return 0;

  if((destroi_lista(g->arestas) & destroi_lista(g->vertices)) == 1) {
    free(g->nome);
    free(g);

    return 1;
  }
  
  return 0;
}

//------------------------------------------------------------------------------
// Aloca a estrutura de um vertice
//
// Recebe como entradada o nome e o tipo do vertice
//
// Devolve o vertice em caso de sucesso, ou NULL, caso contrário
static vertice cria_vertice(char *nome) {
  vertice v = (vertice)malloc(sizeof(struct vertice));
  if(!v)
    err_exit("Erro ao criar vertice!");

  v->nome = (char *)malloc(256*sizeof(char));
  if(!v->nome)
    err_exit("Erro ao criar vertice->nome!");

  strcpy(v->nome, nome);

  v->cor = 0;
  v->rotulo = 0;
  v->visitado = 0;

  v->entrada = cria_lista();
  v->saida = cria_lista();

  return v;
}

//------------------------------------------------------------------------------
// Aloca a estrutura de uma aresta
//
// Recebe como entradada o vertice de origem, o vertice de destino e o peso
//
// Devolve a aresta em caso de sucesso, ou NULL, caso contrário
static aresta_p cria_aresta(vertice origem, vertice destino) {
  aresta_p e = (aresta_p)malloc(sizeof(aresta_t));
  if(!e)
    err_exit("Erro ao criar aresta!");

  e->origem = origem;
  e->destino = destino;

  return e;
}

//------------------------------------------------------------------------------
// Busca uma aresta a no grafo g
//
// Recebe como entradada um grafo e uma aresta
//
// Devolve 1 caso exista a aresta, ou 0, caso contrário
static int busca_aresta (grafo g, aresta_p a) {
  if(!g)
    err_exit("Erro ao carregar o grafo!");

  nodo_lista_p n;

  for (n = g->arestas->primeiro; n != NULL; n = n->proximo) {
    aresta_p e = n->conteudo;

    if ((a->origem == e->origem) && (a->destino == e->destino))
      return 1;
  }

  return 0;
}

//------------------------------------------------------------------------------
// Verifica se existe um vertice v no grafo g
//
// Recebe com entradada um grafo g e um vertice v
//
// Devolve 1 caso exista, ou 0 caso, contrário
static vertice busca_vertice(grafo g, vertice v) {
  if(!g)
    err_exit("Erro ao carregar o grafo!");

  nodo_lista_p nodo_v;
  vertice ve;

  for(nodo_v = g->vertices->primeiro; nodo_v != NULL; nodo_v = nodo_v->proximo) {
    ve = nodo_v->conteudo;

    if(ve == v ) {
      return ve;
    }
  }

  return NULL;
}

static vertice rotulo_maximo(lista_p v) {
  if(!v)
    err_exit("Erro ao carregar a lista!");

  nodo_lista_p nodo;
  vertice ve, vr;
  ve = NULL;
  for(nodo = v->primeiro; nodo != NULL; nodo = nodo->proximo) {
    vr = nodo->conteudo;
    if(!vr->visitado) {
      if(!ve) {
        ve = vr;
      } else if (vr->rotulo > ve->rotulo) {
        ve = vr;
      }
    }
  }

  if(ve)
    ve->visitado = 1;
  return ve;
}

//------------------------------------------------------------------------------
// devolve o número de vértices de g
int n_vertices(grafo g) {
  return (int) g->vertices->tam;
}

//------------------------------------------------------------------------------
// devolve o vértice de nome 'nome' em g
vertice vertice_de_nome(char *nome, grafo g) {
  if(!g)
    err_exit("Erro ao carregar o grafo!");

  nodo_lista_p nodo_v;
  vertice ve;

  for(nodo_v = g->vertices->primeiro; nodo_v != NULL; nodo_v = nodo_v->proximo) {
    ve = nodo_v->conteudo;

    if(strcmp(nome, ve->nome) == 0) {
      return ve;
    }
  }

  return NULL;
}


//------------------------------------------------------------------------------
// lê um grafo no formato dot de input
// 
// devolve o grafo lido,
//         ou 
//         NULL, em caso de erro 

grafo le_grafo(FILE *input) {
  Agraph_t *graph;
  Agnode_t *node;
  Agedge_t *edge;

  nodo_lista_p n;
  aresta_p e;
  vertice destino, origem;

  // Cria o grafo g
  grafo g = cria_grafo();

  // Le o arquivo de entrada
  graph = agread(input, NULL);
  
  // Atribui o nome do grafo
  strcpy(g->nome, agnameof(graph));

  // Adiciona os vertices ao grafo g
  for (node = agfstnode(graph); node != NULL; node = agnxtnode(graph, node)) {
    add_nodo_lista(g->vertices, cria_nodo_lista(cria_vertice(agnameof(node))));
  }

  // Para cada nodo do grafo .dot
  for (node = agfstnode(graph); node != NULL; node = agnxtnode(graph, node)) {
    // Para cada aresta do grafo .dot

    for (edge = agfstedge(graph, node); edge != NULL; edge = agnxtedge(graph, edge, node)) {

      // Verifica se o vertice de g e tail na aresta do grafo .dot
      for (n = g->vertices->primeiro; n != NULL; n = n->proximo) {
        origem = n->conteudo;
        if (strcmp(origem->nome, agnameof(agtail(edge))) == 0)
          break;
      }

      // Verifica se o vertice de g e head na aresta do grafo .dot
      for (n = g->vertices->primeiro; n != NULL; n = n->proximo) {
        destino = n->conteudo;
        if (strcmp(destino->nome, agnameof(aghead(edge))) == 0)
          break;
      }

      // Cria uma aresta {tail,head} com peso 1
      e = cria_aresta(origem, destino);

      // Se a aresta não existe em e
      if (!busca_aresta(g, e)) {

        nodo_lista_p nodo = cria_nodo_lista(e);
        // Adiciona e a lista de arestas de g
        add_nodo_lista(g->arestas, nodo);

        // Adiciona e a lista de arestas de saida do vertice de origem
        add_nodo_lista(origem->saida, nodo);

        // Adiciona e a lista de arestas de entrada do vertice de destino
        add_nodo_lista(destino->entrada, nodo);
      }    
    }
  }
 
  agclose(graph);

  return g;
}
//------------------------------------------------------------------------------
// escreve o grafo g em output usando o formato dot.
//
// devolve o grafo escrito,
//         ou 
//         NULL, em caso de erro 

grafo escreve_grafo(FILE *output, grafo g) {
  if(!g)
    err_exit("Erro ao carregar o grafo!");

  // Imprime o cabecalho do arquivo .dot contendo o nome do grafo
  fprintf(output, "%s ", "strict");
  fprintf(output, "graph %s {", g->nome);

  fprintf(output, "\n");

  // Imprime as arestas do grafo g
  for(nodo_lista_p n = g->arestas->primeiro; n != NULL; n = n->proximo) {
    aresta_p a = n->conteudo;

    fprintf(output, "\t%s ", a->origem->nome);

    fprintf(output, "-- ");
    fprintf(output, "%s", a->destino->nome);
    fprintf(output, "\n");
  }

  fprintf(output, "}\n");

  return g;
}

//------------------------------------------------------------------------------
// devolve um número entre 0 e o número de vertices de g

unsigned int cor(vertice v, grafo g) {
  if(!g)
    err_exit("Erro ao carregar o grafo!");

  vertice ve = busca_vertice(g, v);

  return ve->cor;
}

//------------------------------------------------------------------------------
// preenche o vetor v (presumidamente um vetor com n_vertices(g)
// posições) com os vértices de g ordenados de acordo com uma busca em
// largura lexicográfica sobre g a partir de r e devolve v

vertice *busca_lexicografica(vertice r, grafo g, vertice *v) {
  if(!g)
    err_exit("Erro ao carregar o grafo!");

  nodo_lista_p nodo_u;
  lista_p lista_v;
  vertice ve;
  aresta_p ar;
  unsigned int tam_g = g->vertices->tam;

  ve = busca_vertice(g, r);

  ve->rotulo = tam_g;
  ve->visitado = 1;
  tam_g -= 1;

  v[g->vertices->tam-1] = ve;

  for(nodo_u = g->arestas->primeiro; nodo_u != NULL; nodo_u = nodo_u->proximo) {
    ar = nodo_u->conteudo;

    if(ve == ar->origem) {
      ar->destino->rotulo = tam_g;
    } else if(ve == ar->destino) {
      ar->origem->rotulo = tam_g;
    }
  }

  lista_v = g->vertices;

  while((ve = rotulo_maximo(lista_v)) != NULL) {
    v[tam_g-1] = ve;
    tam_g -= 1;

    for(nodo_u = g->arestas->primeiro; nodo_u != NULL; nodo_u = nodo_u->proximo) {
      ar = nodo_u->conteudo;

      if(ve == ar->origem) {
        if(tam_g > ar->destino->rotulo)
          ar->destino->rotulo = tam_g;
      } else if(ve == ar->destino) {
        if(tam_g > ar->origem->rotulo)
          ar->origem->rotulo = tam_g;
      }
    }
  }

  return v;
}

//------------------------------------------------------------------------------
// colore os vértices de g de maneira "gulosa" segundo a ordem dos
// vértices em v e devolve o número de cores utilizado
//
// ao final da execução,
//     1. cor(v,g) > 0 para todo vértice de g
//     2. cor(u,g) != cor(v,g), para toda aresta {u,v} de g

unsigned int colore(grafo g, vertice *v) {
  if(!g)
    err_exit("Erro ao carregar o grafo!");

  unsigned int i, j, num_cores;
  vertice vr;
  aresta_p ar;
  nodo_lista_p nodo_u;
  int disponivel[50];

  num_cores = 0;
  vr = v[0];
  vr->cor = 1;


  for(i=1; i<g->vertices->tam; i++) {
    vr = v[i];
    vr->cor = 1;
    for(j=0; j<50; j++) disponivel[j] = 1;
    for(nodo_u = g->arestas->primeiro; nodo_u != NULL; nodo_u = nodo_u->proximo) {
      ar = nodo_u->conteudo;
      if(vr == ar->destino) {
        disponivel[ar->origem->cor] = 0;
      } else if(vr == ar->origem) {
        disponivel[ar->destino->cor] = 0;
      }
    }

    for(j=1; j<50; j++) {
      if(disponivel[j]) {
        vr->cor = j;
        break;
      }
    }

    if(num_cores < vr->cor)
      num_cores = vr->cor;
  }

  return num_cores;
}