#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <MLV/MLV_all.h>
#include <string.h>
#define MAX_FILE_LINE_SIZE 100
#define MAX_LINES 16
#define MAX_COLUMNS 50
#define MAX_PATH_LEN 64
#define NB_LEVELS 10

int nb_line = 7;
int nb_column = 24;
int window_width = 0;
int window_height = 0;


/*************************************************************/
/*                           Types                           */
/*************************************************************/


typedef struct defenses {
	int type;
	int life;
	int max_life;
	int damage;
	int line;
	int position;
	int price;
	struct defenses* next;
} Defenses;

typedef struct cats {
	int type;
	int life;
	int max_life;
	int damage;
	int poison;
	int slowness;
	int line;
	int position;
	int speed;
	int turn;
	int id_sprite;
	struct cats* next;
	struct cats* next_line;
	struct cats* prev_line;
} Cats;

typedef struct {
	Cats* cats;
	Defenses* defenses;
	int turn;
	int money;
	int life;
	int level;
	int score;
} Game;


/* Affichage Graphique */


typedef struct {
	MLV_Image* image;
	MLV_Image* shadows;
	int start_x;
	int start_y;
	int path_size;
	int case_width;
	int case_height;
} Background;

typedef struct {
	int type;
	MLV_Image* images[3];
	int nb_images;
} Sprite;

typedef struct {
	Background bg;
	Sprite sprites_cats[11];
	Sprite sprites_defs[8];
	MLV_Image* sprites_defs_menu[8];
	int size_sprites;
	MLV_Font* fonts[5];
	int nb_fonts;
	MLV_Image* chabsolu[3];
	MLV_Image* pattounes;
	MLV_Image* menu;
	MLV_Image* loading;
	MLV_Image* bg_menu;
	MLV_Image* catpedia[19];
	MLV_Image* win;
	MLV_Image* lost;
} Assets;

typedef struct {
	MLV_Image* frames[16];
	int nb_frames;
} AnimCatDeath;

typedef struct {
	AnimCatDeath cat_death;
	MLV_Image* frames[16];
	int nb_frames;
} Animations;


/*************************************************************/
/*                        Prototypes                         */
/*************************************************************/


void deleteDead(Game* g, Assets* assets, Animations* anim, int graphic, int is_infinite);
void deleteDeadDefenses(Game* g);
void deleteDeadCats(Game* g, int is_infinite);
void firstCatOfLines(Game g, Cats* first_cats[]);


/*************************************************************/
/*                   Utilitaires Global                      */
/*************************************************************/


int min(int a, int b) {
	return (a <= b) ? a : b;
}

int max(int a, int b) {
	return (a >= b) ? a : b;
}

void delay(int nb_ms) {
	int nb = 1000 * nb_ms;
	time_t start = clock();
	while(clock() < start + nb);
}


/*************************************************************/
/*                         Défenses                          */
/*************************************************************/


int initDefense(Defenses* defense, int line, int position, char type) {
	defense->type = type;
	defense->line = line;
	defense->position = position;
	switch (type) {
		case 'o':
			defense->life = 100;
			defense->max_life = 100;
			defense->price = 100;
			defense->damage = 8;
			return defense->price;
		case ']':
			defense->life = 400;
			defense->max_life = 400;
			defense->price = 100;
			defense->damage = 0;
			return defense->price;
		case '(':
			defense->life = 1;
			defense->max_life = 1;
			defense->price = 50;
			defense->damage = 80;
			return defense->price;
		case '>':
			defense->life = 120;
			defense->max_life = 120;
			defense->price = 200;
			defense->damage = 2;
			return defense->price;
		case '_':
			defense->life = 300;
			defense->max_life = 300;
			defense->price = 100;
			defense->damage = 1;
			return defense->price;
		case '@':
			defense->life = 150;
			defense->max_life = 150;
			defense->price = 350;
			defense->damage = 12;
			return defense->price;
		case '{':
			defense->life = 90;
			defense->max_life = 90;
			defense->price = 150;
			defense->damage = 4;
			return defense->price;
		case '&':
			defense->life = 100;
			defense->max_life = 100;
			defense->price = 50;
			defense->damage = 0;
			return defense->price;
		default:
			return 0;
	}
}

int addDefense(Game* g, int line, int position, char type, int hasCost) {
	Defenses* defense = (Defenses*) malloc(sizeof(Defenses));
	Defenses* tmp = g->defenses;
	int price;

	if (defense == NULL) return 1;

	price = initDefense(defense, line, position, type);
	if (price == 0) {
		free(defense);
		return 1;
	}
	if (hasCost) {
		if (price > g->money) {
			free(defense);
			return 2;
		}
		g->money -= price;
	}

	defense->next = NULL;

	if (g->defenses == NULL) {
		g->defenses = defense;
		return 0;
	}

	while (tmp->next) {
		tmp = tmp->next;
	}
	tmp->next = defense;

	return 0;
}

int existDefense(Game g, int line, int column) {
	Defenses* tmp = g.defenses;
	while (tmp) {
		if (tmp->line == line && tmp->position == column) {
			return 1;
		}
		tmp = tmp->next;
	}
	return 0;
}

void deleteDefenseCoord(Game* g, int line, int column) {
	Defenses* tmp = g->defenses;
	Defenses* tmp2;
	if (tmp->line == line && tmp->position == column) {
		g->money += tmp->price / 2;
		g->defenses = tmp->next;
		free(tmp);
		return;
	}
	while (tmp->next) {
		if (tmp->next->line == line && tmp->next->position == column) {
			tmp2 = tmp->next;
			g->money += tmp2->price / 2;
			tmp->next = tmp2->next;
			free(tmp2);
			return;
		}
		tmp = tmp->next;
	}
}


/* Actions */


void actionBallLauncher(Game* g, Defenses* ball_launcher) {
	Cats* tmp;
	Cats* first_cats[MAX_LINES];
	firstCatOfLines(*g, first_cats);
	tmp = first_cats[ball_launcher->line - 1];
	while (tmp) {
		if (tmp->type != '.' && tmp->position > ball_launcher->position && tmp->position <= nb_column) {
			tmp->life -= ball_launcher->damage;
			return;
		}
		tmp = tmp->next_line;
	}
}

void actionLaser(Game* g, Defenses* laser) {
    Cats* tmp = g->cats;
    while (tmp) {
        if (tmp->line == laser->line && tmp->position > laser->position && tmp->position <= nb_column) {
        	if (tmp->type != '.') tmp->life -= laser->damage;
            tmp = tmp->next_line;
        }
        else {
        tmp = tmp->next;
        }
    }
}

void actionCucumber(Game* g, Defenses* cucumber) {
	Cats* tmp_cat = g->cats;
	Defenses* tmp_def = g->defenses;
	while (tmp_cat) {
		if (cucumber->line - 1 <= tmp_cat->line && tmp_cat->line <= cucumber->line + 1 && cucumber->position - 1 <= tmp_cat->position && tmp_cat->position <= cucumber->position + 1) {
			tmp_cat->life -= cucumber->damage;
		}
		tmp_cat = tmp_cat->next;
	}
	while (tmp_def) {
		if (cucumber->line - 1 <= tmp_def->line && tmp_def->line <= cucumber->line + 1 && cucumber->position - 1 <= tmp_def->position && tmp_def->position <= cucumber->position + 1) {
			tmp_def->life -= cucumber->damage / 2;
		}
		tmp_def = tmp_def->next;
	}
}

void actionPuddle(Game* g, Defenses* puddle, Cats* cat) {
	cat->slowness = 11;
	cat->life -= puddle->damage;
	puddle->life -= 10;
}

void actionDog(Game* g, Defenses* dog) {
	Cats* tmp = g->cats;
	while (tmp) {
		if (tmp->type != '<') {
			if (dog->line - 1 <= tmp->line && tmp->line <= dog->line + 1 && dog->position - 1 <= tmp->position && tmp->position <= dog->position + 2 && tmp->position <= nb_column) {
				tmp->life -= dog->damage / 2;
			}
			if (dog->line - 1 <= tmp->line && tmp->line <= dog->line + 1 && dog->position - 1 <= tmp->position && tmp->position <= dog->position + 1 && tmp->position <= nb_column) {
				tmp->life -= dog->damage / 2;
			}
		}
		tmp = tmp->next;
	}
}

void actionSprinkler(Game* g, Defenses* sprinkler) {
    Cats* tmp;
    Cats* first_cats[MAX_LINES];
    int i;
    firstCatOfLines(*g, first_cats);
    for(i = -1; i <= 1; i++){
        if (sprinkler->line - 1 + i < nb_line && sprinkler->line - 1 + i >= 0){
            tmp = first_cats[sprinkler->line - 1 + i];
            while (tmp) {
                if (tmp->type != '.' && tmp->position > sprinkler->position && tmp->position <= nb_column) {
                    tmp->life -= sprinkler->damage;
                    break;
                }
                tmp = tmp->next_line;
            }
        }
    }
}

void actionFish(Game* g, Defenses* fish, Cats* cat) {
	cat->poison += 5;
	fish->life -= 10;
}

void actionDefenses(Game* g) {
	Defenses* tmp = g->defenses;
	while (tmp) {
		switch (tmp->type) {
			case 'o':
				actionBallLauncher(g, tmp);
				break;
			case '>':
				actionLaser(g, tmp);
				break;
			case '{':
				actionSprinkler(g, tmp);
				break;
			case '@':
				actionDog(g, tmp);
				break;
			default:
				break;
		}
		tmp = tmp->next;
	}
}


/* Suppression */


Defenses* deleteFirstDefense(Game* g, Defenses* def) {
	g->defenses = def->next;
	free(def);
	return g->defenses;
}

Defenses* deleteDefense(Game* g, Defenses* def) {
	Defenses* tmp = def->next;
	def->next = tmp->next;
	free(tmp);
	return def;
}

void deleteDeadDefenses(Game* g) {
	Defenses* tmp = g->defenses;

	while(tmp) {
		if (tmp->life <= 0) {
			if (tmp->type == '(') actionCucumber(g, tmp);
			tmp = deleteFirstDefense(g, tmp);
		} else {
			break;
		}
	}

	while (tmp && tmp->next) {
		if (tmp->next->life <= 0) {
			if (tmp->next->type == '(') actionCucumber(g, tmp->next);
			tmp = deleteDefense(g, tmp);
		} else {
			tmp = tmp->next;
		}
	}
}


/*************************************************************/
/*                           Chats                           */
/*************************************************************/


void initCat(Cats* cat, int turn, int line, char type) {
	cat->type = type;
	cat->line = line;
	cat->turn = turn;
	cat->position = nb_column + turn;
	cat->poison = 0;
	cat->slowness = 0;
	switch (type) {
		case 'C':
			cat->life = 100;
			cat->max_life = 100;
			cat->speed = 4;
			cat->damage = 20;
			break;
		case 'c':
			cat->life = 50;
			cat->max_life = 50;
			cat->speed = 8;
			cat->damage = 10;
			break;
		case '#':
			cat->life = 250;
			cat->max_life = 250;
			cat->speed = 2;
			cat->damage = 30;
			break;
		case '+':
			cat->life = 80;
			cat->max_life = 80;
			cat->speed = 4;
			cat->damage = 10;
			break;
		case 'x':
			cat->life = 40;
			cat->max_life = 40;
			cat->speed = 12;
			cat->damage = 150;
			break;
		case '<':
			cat->life = 80;
			cat->max_life = 80;
			cat->speed = 4;
			cat->damage = 0;
			break;
		case '^':
			cat->life = 100;
			cat->max_life = 100;
			cat->speed = 4;
			cat->damage = 5;
			break;
		case '.':
			cat->life = 80;
			cat->max_life = 80;
			cat->speed = 4;
			cat->damage = 15;
			break;
		case 'A':
			cat->life = 350;
			cat->max_life = 350;
			cat->speed = 2;
			cat->damage = 25;
			break;
		case '*':
			cat->life = 100;
			cat->max_life = 100;
			cat->speed = 4;
			cat->damage = 30;
			break;
		default:
			break;
	}
}

int addCat(Game* g, int turn, int line, char type) {
	Cats* cat = (Cats*) malloc(sizeof(Cats));
	Cats* tmp = g->cats;
	Cats* tmp2;
	int line_chain = 0;

	if (cat == NULL) return 1;

	initCat(cat, turn, line, type);
	cat->next = NULL;
	cat->next_line = NULL;

	if (g->cats == NULL) {
		cat->prev_line = NULL;
		g->cats = cat;
		return 0;
	}

	if (tmp->next == NULL && tmp->line == line) {
		tmp->next_line = cat;
		cat->prev_line = tmp;
		line_chain = 1;
	}

	while (tmp->next) {
		if (!line_chain && tmp->line == line) {
			tmp2 = tmp;
			while (tmp2->next_line) {
				tmp2 = tmp2->next_line;
			}
			tmp2->next_line = cat;
			cat->prev_line = tmp2;
			line_chain = 1;
		}
		tmp = tmp->next;
	}

	if (!line_chain && tmp->line == line) {
		tmp2 = tmp;
		while (tmp2->next_line) {
			tmp2 = tmp2->next_line;
		}
		tmp2->next_line = cat;
		cat->prev_line = tmp2;
		line_chain = 1;
	}

	if (!line_chain) {
		cat->prev_line = NULL;
		line_chain = 1;
	}
	tmp->next = cat;

	return 0;
}

void firstCatOfLines(Game g, Cats* first_cats[]) {
	int i;
	Cats* tmp;
	for (i = 1; i <= nb_line; i++) {
		first_cats[i - 1] = NULL;
	}
	if (g.cats == NULL) return;
	for (i = 1; i <= nb_line; i++) {
		tmp = g.cats;
		while (tmp) {
			if (tmp->line == i) {
				first_cats[i - 1] = tmp;
				break;
			}
			tmp = tmp->next;
		}
	}
}


/* Déplacement */


int moveCat(Cats* cat, int speed, int turn) {
	int nb_case;
	if (cat->prev_line == NULL) {
		nb_case = speed;
	} else {
		nb_case = min(4 * (cat->position - cat->prev_line->position - 1), speed);
	}
	if (nb_case == 1) {
		if ((turn - cat->turn) % 4 == 0) {
			cat->position -= 1;
			return 1;
		}
	} else if (nb_case == 2) {
		if ((turn - cat->turn) % 2 == 0) {
			cat->position -= 1;
			return 1;
		}
	} else {
		cat->position -= nb_case / 4;
	}
	return nb_case / 4;
}

void actionOnCatPath(Game* g, Cats* cat, int nb_cases) {
	Defenses* tmp = g->defenses;
	if (cat->type == '<') return;
	while (tmp) {
		if (tmp->line == cat->line && ((tmp->position >= cat->position && tmp->position < cat->position + nb_cases) || (nb_cases == 0 && tmp->position == cat->position))) {
			if (tmp->type == '_') {
				actionPuddle(g, tmp, cat);
			}
			if (tmp->type == '(') {
				actionCucumber(g, tmp);
			}
			if (tmp->type == '&') {
				actionFish(g, tmp, cat);
			}
		}
		tmp = tmp->next;
	}
}

void moveIncomingCats(Game* g) {
	Cats* tmp = g->cats;
	int nb_cases;
	while (tmp) {
		if (tmp->position > nb_column) {
			nb_cases = moveCat(tmp, 4, g->turn);
			actionOnCatPath(g, tmp, nb_cases);
		}
		tmp = tmp->next;
	}
}

int distanceToDefense(Game g, Cats* cat) {
	Defenses* tmp = g.defenses;
	int distance = cat->position;
	if (cat->type == '<') return distance;
	while (tmp) {
		if (tmp->line == cat->line && tmp->position < cat->position) {
			if (tmp->type != '_' && tmp->type != '(' && tmp->type != '&') {
				if (cat->position - tmp->position - 1 < distance) {
					distance = cat->position - tmp->position - 1;
					if (distance == 0) return 0;
				}
			}
		}
		tmp = tmp->next;
	}
	return distance;
}

void moveVisibleCats(Game* g) {
	Cats* tmp = g->cats;
	int nb_cases, speed;
	while (tmp) {
		if (tmp->position <= nb_column) {
			speed = min(tmp->speed, 4 * distanceToDefense(*g, tmp));
			if (tmp->slowness > 0) {
				if (speed == 4) speed = 2;
				else if (speed == 2) speed = 1;
				else if ((speed / 4) % 2 == 0) speed /= 2;
				else speed = speed / 2 + 2;
			}
			nb_cases = moveCat(tmp, speed, g->turn);
			actionOnCatPath(g, tmp, nb_cases);
		}
		tmp = tmp->next;
	}
}


/* Actions */


void actionCharurgien(Game* g, Cats* charurgien) {
	Cats* tmp_cat = g->cats;
	while (tmp_cat) {
		if (tmp_cat == charurgien) {
			tmp_cat = tmp_cat->next;
			continue;
		}
		if (charurgien->line - 1 <= tmp_cat->line && tmp_cat->line <= charurgien->line + 1 && charurgien->position - 1 <= tmp_cat->position && tmp_cat->position <= charurgien->position + 1) {
			tmp_cat->life = min(tmp_cat->max_life, tmp_cat->life + tmp_cat->max_life * 5 / 100);
		}
		tmp_cat = tmp_cat->next;
	}
}

void actionChamikaze(Game* g, Cats* chamikaze) {
	Cats* tmp_cat = g->cats;
	Defenses* tmp_def = g->defenses;
	while (tmp_cat) {
		if (chamikaze->line - 1 <= tmp_cat->line && tmp_cat->line <= chamikaze->line + 1 && chamikaze->position - 1 <= tmp_cat->position && tmp_cat->position <= chamikaze->position + 1) {
			tmp_cat->life -= chamikaze->damage / 2;
		}
		tmp_cat = tmp_cat->next;
	}
	while (tmp_def) {
		if (chamikaze->line - 1 <= tmp_def->line && tmp_def->line <= chamikaze->line + 1 && chamikaze->position - 1 <= tmp_def->position && tmp_def->position <= chamikaze->position + 1) {
			tmp_def->life -= chamikaze->damage;
		}
		tmp_def = tmp_def->next;
	}
}

void actionChagicien(Game* g, Cats* chagicien) {
	Defenses* target = NULL;
	Defenses* tmp = g->defenses;

	while (tmp) {
		if (tmp->type != '_' && tmp->type != '&') {
			if (tmp->line == chagicien->line && tmp->position < chagicien->position) {
				if (target == NULL) target = tmp;
				else {
					if (tmp->position > target->position) {
						target = tmp;
					}
				}
			}
		}
		tmp = tmp->next;
	}
	if (target)	target->life -= chagicien->damage;
}

void actionChatomic(Game* g, Cats* chatomic) {
	Cats* tmp_cat = g->cats;
	Defenses* tmp_def = g->defenses;
	while (tmp_cat) {
		if (chatomic->line - 1 <= tmp_cat->line && tmp_cat->line <= chatomic->line + 1 && chatomic->position - 1 <= tmp_cat->position && tmp_cat->position <= chatomic->position + 1) {
			tmp_cat->life -= chatomic->damage * 4;
		}
		if (chatomic->line - 2 <= tmp_cat->line && tmp_cat->line <= chatomic->line + 2 && chatomic->position - 2 <= tmp_cat->position && tmp_cat->position <= chatomic->position + 2) {
			tmp_cat->life -= chatomic->damage * 3;
		}
		if (chatomic->line - 3 <= tmp_cat->line && tmp_cat->line <= chatomic->line + 3 && chatomic->position - 3 <= tmp_cat->position && tmp_cat->position <= chatomic->position + 3) {
			tmp_cat->life -= chatomic->damage * 2;
		}
		tmp_cat = tmp_cat->next;
	}
	while (tmp_def) {
		if (chatomic->line - 1 <= tmp_def->line && tmp_def->line <= chatomic->line + 1 && chatomic->position - 1 <= tmp_def->position && tmp_def->position <= chatomic->position + 1) {
			tmp_def->life -= chatomic->damage * 4;
		}
		if (chatomic->line - 2 <= tmp_def->line && tmp_def->line <= chatomic->line + 2 && chatomic->position - 2 <= tmp_def->position && tmp_def->position <= chatomic->position + 2) {
			tmp_def->life -= chatomic->damage * 3;
		}
		if (chatomic->line - 3 <= tmp_def->line && tmp_def->line <= chatomic->line + 3 && chatomic->position - 3 <= tmp_def->position && tmp_def->position <= chatomic->position + 3) {
			tmp_def->life -= chatomic->damage * 2;
		}
		tmp_def = tmp_def->next;
	}
}

void actionChapin(Game* g, Cats* chapin) {
	char types_defenses[8] = {'o', ']', '(', '>', '_', '@', '{', '&'};
	if (!existDefense(*g, chapin->line, chapin->position) && 1 <= chapin->position && chapin->position <= nb_column - 1) {
		addDefense(g, chapin->line, chapin->position, types_defenses[rand() % 8], 0);
	}
}

void actionChabsolu(Game* g) {
	int i;
	Cats* tmp;
	Cats* first_cats[MAX_LINES];
	firstCatOfLines(*g, first_cats);
	for (i = 0; i < nb_line; i++) {
		tmp = first_cats[i];
		if (tmp) {
			if (tmp->position <= 0) {
				while (tmp) {
					tmp->life = 0;
					tmp = tmp->next_line;
				}
			} else {
				tmp->life = 0;
			}
		}
	}
}

void attackCat(Game* g, Cats* cat) {
	Defenses* tmp = g->defenses;
	while (tmp) {
		if (tmp->type != '_' && tmp->type != '&') {
			if (tmp->line == cat->line && tmp->position == cat->position - 1) {
				if (cat->type == 'x') actionChamikaze(g, cat);
				else tmp->life -= cat->damage;
			}
		}
		tmp = tmp->next;
	}
}

void actionCats(Game* g) {
	Cats* tmp = g->cats;
	while (tmp) {
		if (tmp->position > nb_column) {
			tmp = tmp->next;
			continue;
		}
		if (tmp->slowness > 0) tmp->slowness--;
		if (tmp->poison > 0) {
			tmp->life -= 10;
			tmp->poison--;
		}

		if (tmp->type != '<' && tmp->type != '^') {
			attackCat(g, tmp);
		}

		switch (tmp->type) {
			case '+':
				actionCharurgien(g, tmp);
				break;
			case '^':
				actionChagicien(g, tmp);
				break;
			default:
				break;
		}
		tmp = tmp->next;
	}
}


/* Suppression */


Cats* deleteFirstCat(Game* g, Cats* cat) {
	Cats* tmp;
	/* Seul */
	if (cat->next == NULL) {
		g->cats = NULL;
		free(cat);
		return NULL;
	}
	tmp = cat->next;
	g->cats = cat->next;
	/* Premier du chainage simple */
	if (cat->prev_line == NULL) {
		/* Seul sur sa ligne */
		if (cat->next_line == NULL) {
			free(cat);
			return tmp;
		}
		/* Premier sur sa ligne */
		cat->next_line->prev_line = NULL;
		free(cat);
		return tmp;
	}
	/* Dernier sur sa ligne */
	if (cat->next_line == NULL) {
		cat->prev_line->next_line = NULL;
		free(cat);
		return tmp;
	}
	/* Au milieu de la ligne */
	cat->next_line->prev_line = cat->prev_line;
	cat->prev_line->next_line = cat->next_line;
	free(cat);
	return tmp;
}

Cats* deleteCat(Game* g, Cats* cat) {
	Cats* tmp;

	tmp = cat->next;
	/* Dernier du chainage simple */
	if (cat->next->next == NULL) {
		if (cat->next->prev_line == NULL) {
			/* Seul sur sa ligne */
			if (cat->next->next_line == NULL) {
				cat->next = NULL;
				free(tmp);
				return NULL;
			}
			/* Premier sur sa ligne */
			tmp->next_line->prev_line = NULL;
			cat->next = NULL;
			free(tmp);
			return NULL;
		}
		/* Dernier sur sa ligne */
		if (cat->next->next_line == NULL) {
			tmp->prev_line->next_line = NULL;
			cat->next = NULL;
			free(tmp);
			return NULL;
		}
		/* Au milieur de sa ligne */
		tmp->next_line->prev_line = tmp->prev_line;
		tmp->prev_line->next_line = tmp->next_line;
		cat->next = NULL;
		free(tmp);
		return NULL;
	}

	/* Au milieu du chainage simple */
	if (cat->next->prev_line == NULL) {
		/* Seul sur sa ligne */
		if (cat->next->next_line == NULL) {
			cat->next = tmp->next;
			free(tmp);
			return cat;
		}
		/* Premier sur sa ligne */
		tmp->next_line->prev_line = NULL;
		cat->next = tmp->next;
		free(tmp);
		return cat;
	}
	/* Dernier sur sa ligne */
	if (cat->next->next_line == NULL) {
		tmp->prev_line->next_line = NULL;
		cat->next = tmp->next;
		free(tmp);
		return cat;
	}
	/* Au milieu de sa ligne */
	tmp->next_line->prev_line = tmp->prev_line;
	tmp->prev_line->next_line = tmp->next_line;
	cat->next = tmp->next;
	free(tmp);
	return cat;
}

void deleteDeadCats(Game* g, int is_infinite) {
	Cats* tmp = g->cats;

	while(tmp) {
		if (tmp->life <= 0) {
			if (tmp->type == 'A') actionChatomic(g, tmp);
			if (tmp->type == '*') actionChapin(g, tmp);
			if (is_infinite) g->money += tmp->max_life / 5;
			g->score += tmp->position + tmp->max_life / 10;
			tmp = deleteFirstCat(g, tmp);
		} else {
			break;
		}
	}

	while (tmp && tmp->next) {
		if (tmp->next->life <= 0) {
			if (tmp->next->type == 'A') actionChatomic(g, tmp->next);
			if (tmp->next->type == '*') actionChapin(g, tmp->next);
			if (is_infinite) g->money += tmp->max_life / 5;
			g->score += tmp->position + tmp->max_life / 10;
			tmp = deleteCat(g, tmp);
		} else {
			tmp = tmp->next;
		}
	}
}


/*************************************************************/
/*                     Affichage ASCII                       */
/*************************************************************/


void printWaveLine(Cats* first_cat) {
	int j, j_max;

	while (first_cat) {
		if (first_cat->prev_line) {
			j_max = first_cat->turn - first_cat->prev_line->turn - 1;
		}
		else {
			j_max = first_cat->turn - 1;
		}
		
		for (j = 0; j < j_max; j++) {
			printf("  ");
		}
		printf(" %c", first_cat->type);
		first_cat = first_cat->next_line;
	}
	printf("\n");
	return;
}

void printWave(Game g) {
	int i;
	Cats* first_cats[MAX_LINES];
	firstCatOfLines(g, first_cats);
	printf("\nUne armee de chats approche...\n");
	for (i = 1; i <= nb_line; i++) {
		printf("%d|", i);
		printWaveLine(first_cats[i - 1]);
	}
}

void printDefenses(Game g) {
	int i, j;
	Defenses* tmp = g.defenses;
	char defenses_line[MAX_LINES][MAX_COLUMNS * 2 + 2];

	for (i = 0; i < nb_line; i++) {
		defenses_line[i][0] = (char) (i + 49);
		defenses_line[i][1] = '|';
		for (j = 2; j < nb_column * 2; j++) {
			defenses_line[i][j] = ' ';
		}
		defenses_line[i][nb_column * 2] = '|';
		defenses_line[i][nb_column * 2 + 1] = (char) (i + 49);
		defenses_line[i][nb_column * 2 + 2] = '\0';
	}
	
	while (tmp) {
		defenses_line[tmp->line - 1][tmp->position * 2 + 1] = tmp->type;
		tmp = tmp->next;
	}

	printf("\nVoici vos defense :\n");

	printf(" |");
	for (i = 1; i <= nb_column - 1; i++) {
		if (i % 2 != 0) printf("%2d", i);
		else printf("  ");
	}
	printf("| ");
	
	printf("\n");
	for (i = 0; i < nb_column * 2 + 2; i++) {
		printf("=");
	}
	printf("\n");
	
	for (i = 0; i < nb_line; i++) {
		printf("%s\n", defenses_line[i]);
	}

	for (i = 0; i < nb_column * 2 + 2; i++) {
		printf("=");
	}

	printf("\n |");
	for (i = 1; i <= nb_column - 1; i++) {
		if (i % 2 == 0) printf("%2d", i);
		else printf("  ");
	}
	printf("| \n");
}

void placeDefensesASCII(Game* g) {
	char type;
	int line, column, ret_def = 0;
	do {
		printf("\e[1;1H\e[2J");

		printWave(*g);
		printDefenses(*g);

		switch (ret_def) {
			case 1:
				printf("\nLa defense saisie est invalide.\n");
				break;
			case 2:
				printf("\nVous n'avez pas assez de pattounes.\n");
				break;
			case 3:
				printf("\nL'emplacement est hors du champ de bataille.\n");
				break;
			case 4:
				printf("\nUne defense est deja ici.\n");
				break;
			case 5:
				printf("\nIl n'y a rien a detruire ici.\n");
				break;
		}

		printf("\nVous avez %d pattounes. Que voulez-vous construire ?\n", g->money);
		printf("   o : Lance-Balles (100p)\n");
		printf("   ] : Porte (100p)\n");
		printf("   ( : Concombre (50p)\n");
		printf("   > : Laser (200p)\n");
		printf("   _ : Flaque d'Eau (100p)\n");
		printf("   @ : Chien (350p)\n");
		printf("   { : Arroseur Automatique (150p)\n");
		printf("   & : Poisson Empoisonné (50p)\n");
		printf("   x : NON ! Je veux detruire\n");
		printf("   / : Rien\n");

		printf("\n ==> ");
		scanf(" %c", &type);

		if (type == '/') return;

		if (type == 'x') printf("\nQuel endroit souhaitez-vous liberer ?\n");
		else printf("\nOu voulez-vous placer la defense %c ?\n", type);
		
		printf("Ligne (1 - %d) : ", nb_line);
		scanf(" %d", &line);
		printf("Colonne (1 - %d) : ", nb_column - 1);
		scanf(" %d", &column);

		if (!(1 <= line && line <= nb_line && 1 <= column && column <= nb_column - 1)) {
			ret_def = 3;
			continue;
		}

		if (type == 'x') {
			if (!existDefense(*g, line, column)) {
				ret_def = 5;
				continue;
			}

			deleteDefenseCoord(g, line, column);

		} else {
			if (existDefense(*g, line, column)) {
				ret_def = 4;
				continue;
			}

			ret_def = addDefense(g, line, column, type, 1);
		}

	} while(1);
}

void printTurn(Game g) {
	int i, j, life;
	Defenses* tmp_def = g.defenses;
	Cats* tmp_cat = g.cats;
	char line[MAX_LINES][MAX_COLUMNS * 4 + 3];

	printf("Tour %d\n\n", g.turn);

	for (i = 0; i < nb_line; i++) {
		line[i][0] = (char) (i + 49);
		line[i][1] = '|';
		for (j = 2; j < nb_column * 4 + 2; j++) {
			line[i][j] = ' ';
		}
		line[i][nb_column * 4 + 2] = '|';
		line[i][nb_column * 4 + 3] = '\0';
	}

	while (tmp_def) {
		life = tmp_def->life / 10;

		if (life / 10 == 0) line[tmp_def->line - 1][tmp_def->position * 4 - 1] = ' ';
		else line[tmp_def->line - 1][tmp_def->position * 4 - 1] = (char) life / 10 + '0';

		line[tmp_def->line - 1][tmp_def->position * 4] = (char) life % 10 + '0';

		line[tmp_def->line - 1][tmp_def->position * 4 + 1] = tmp_def->type;
		tmp_def = tmp_def->next;
	}

	while (tmp_cat) {
		if (tmp_cat->position > nb_column) {
			tmp_cat = tmp_cat->next;
			continue;
		}

		life = tmp_cat->life / 10;

		if (life / 10 == 0) line[tmp_cat->line - 1][tmp_cat->position * 4 - 1] = ' ';
		else line[tmp_cat->line - 1][tmp_cat->position * 4 - 1] = (char) life / 10 + '0';

		line[tmp_cat->line - 1][tmp_cat->position * 4] = (char) life % 10 + '0';

		line[tmp_cat->line - 1][tmp_cat->position * 4 + 1] = tmp_cat->type;
		
		tmp_cat = tmp_cat->next;
	}

	for (i = 0; i < nb_column * 4 + 3; i++) {
		printf("=");
	}
	printf("\n");
	for (i = 0; i < nb_line; i++) {
		printf("%s\n", line[i]);
	}
	for (i = 0; i < nb_column * 4 + 3; i++) {
		printf("=");
	}
	printf("\n");
}


/*************************************************************/
/*                       Affichage MLV                       */
/*************************************************************/


/* Utilitaires */


void createWindow() {
	window_width = MLV_get_desktop_width() * 80 / 100;
	window_height = window_width * 9 / 16;
	MLV_create_window("Chattack", "Chattack", window_width, window_height);
}

void resizeSprites(Assets* assets) {
	int i, j, size;

	size = min(assets->bg.case_width, assets->bg.case_height);
	assets->size_sprites = size;

	MLV_resize_image_with_proportions(assets->pattounes, 15 * window_width / 1344, 15 * window_width / 1344);

	for (i = 0; i < 11; i++) {
		for (j = 0; j < assets->sprites_cats[i].nb_images; j++) {
			MLV_resize_image_with_proportions(assets->sprites_cats[i].images[j], size, size);
		}
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < assets->sprites_defs[i].nb_images; j++) {
			MLV_resize_image_with_proportions(assets->sprites_defs[i].images[j], size, size);
		}
		if (assets->sprites_defs_menu[i])
			MLV_resize_image_with_proportions(assets->sprites_defs_menu[i], window_height / 16, window_height / 16);
	}

	for (i = 0; i < 3; i++) {
		MLV_resize_image_with_proportions(assets->chabsolu[i], assets->bg.start_x * 2 / 3, assets->bg.start_x * 2 / 3);
	}
}

void posToCoord(Assets* assets, int line, int column, int* x, int* y) {
	*x = assets->bg.start_x + (column - 1) * assets->bg.case_width;
	*y = assets->bg.start_y + (line * 2 - 1) * (assets->bg.path_size / 2) - assets->size_sprites / 2;
}

void resizeAnim(Assets* assets, Animations* anim) {
	int i, size;

	size = min(assets->bg.case_width, assets->bg.case_height);

	for (i = 0; i < anim->cat_death.nb_frames; i++) {
		MLV_resize_image_with_proportions(anim->cat_death.frames[i], size, size);
	}
}

void resizeMenu(Assets* assets) {
	int i;

	MLV_resize_image_with_proportions(assets->menu, window_width, window_height);
	MLV_resize_image_with_proportions(assets->loading, window_width, window_height);
	MLV_resize_image_with_proportions(assets->bg_menu, window_width, window_height);

	MLV_resize_image_with_proportions(assets->win, window_width, window_height);
	MLV_resize_image_with_proportions(assets->lost, window_width, window_height);

	for (i = 0; i < 19; i++) {
		MLV_resize_image_with_proportions(assets->catpedia[i], window_width, window_height);
	}
}

int switchFullScreen(int is_full_screen) {
	if (is_full_screen) {
		window_width = MLV_get_desktop_width() * 80 / 100;
		window_height = window_width * 9 / 16;
		MLV_create_window("Chattack", "Chattack", window_width, window_height);
	} else {
		window_width = MLV_get_desktop_width();
		window_height = window_width * 9 / 16;
		MLV_create_full_screen_window("Chattack", "Chattack", window_width, window_height);
	}
	return 1 - is_full_screen;
}


/* Chargement des textures */


void loadBackground(Background* bg, char name[]) {
	int i;
	MLV_Image* path;
	MLV_Image* finish;

	if (bg->image) {
		MLV_free_image(bg->image);
		MLV_free_image(bg->shadows);
	}

	bg->start_y = (window_height * 1 / 10);
	bg->path_size = (window_height - bg->start_y) / nb_line;
	bg->case_height = min((window_height * 1 / 5), ((window_height * 9 / 10) / nb_line * 8 / 10));

	path = MLV_load_image("./data/Images/Background/Plains/path.png");
	MLV_resize_image(path, window_width, bg->case_height);

	bg->image = MLV_load_image("./data/Images/Background/Plains/back.png");
	MLV_resize_image_with_proportions(bg->image, window_width, window_height);

	finish = MLV_load_image("./data/Images/Background/Plains/finish.png");
	MLV_resize_image_with_proportions(finish, -1, window_height);

	bg->start_x = MLV_get_image_width(finish) - 10 * nb_line;
	bg->case_width = (window_width - bg->start_x) / nb_column;

	bg->shadows = MLV_load_image("./data/Images/Background/Plains/shadows.png");
	MLV_resize_image_with_proportions(bg->shadows, window_width, window_height);

	for (i = 0; i < nb_line; i++) {
		MLV_draw_image_on_image(path, bg->image, 0, bg->start_y + i * bg->path_size + bg->path_size / 2 - bg->case_height / 2);
	}

	MLV_draw_image_on_image(finish, bg->image, -10 * nb_line, 0);

	MLV_free_image(path);
	MLV_free_image(finish);
}

void loadCatsSprites(Assets* assets) {
	char names[11][16] = {"chat", "chaton", "chaluminium", "charurgien", "chamikaze", "chavion", "chagicien", "chabyssal", "chatomic", "chapin", "charcenciel"};
	char types[11] = {'C', 'c', '#', '+', 'x', '<', '^', '.', 'A', '*', 'U'};
	int i, j;
	char path[MAX_PATH_LEN];
	MLV_Image* tmp;

	for (i = 0; i < 11; i++) {
		j = 0;
		while (1) {
			sprintf(path, "./data/Images/Cats/%s/%d.png", names[i], j + 1);
			tmp = MLV_load_image(path);
			if (tmp == NULL) break;
			assets->sprites_cats[i].images[j] = tmp;
			j++;
		}
		assets->sprites_cats[i].nb_images = j;
		assets->sprites_cats[i].type = types[i];
	}
}

void assignCatSprites(Game* g, Assets* assets) {
	int i;
	Cats* tmp = g->cats;

	while (tmp) {
		for (i = 0; i < 11; i++) {
			if (assets->sprites_cats[i].type == tmp->type) {
				if (assets->sprites_cats[i].nb_images > 0)
				tmp->id_sprite = rand() % assets->sprites_cats[i].nb_images;
				break;
			}
		}
		tmp = tmp->next;
	}
}

void loadDefensesSprites(Assets* assets) {
	char names[8][16] = {"ball_launcher", "door", "cucumber", "laser", "puddle", "dog", "sprinkler", "fish"};
	char types[8] = {'o', ']', '(', '>', '_', '@', '{', '&'};
	int i;
	char path[MAX_PATH_LEN];
	MLV_Image* tmp;

	for (i = 0; i < 8; i++) {
		sprintf(path, "./data/Images/Defenses/%s/1.png", names[i]);
		tmp = MLV_load_image(path);
		if (tmp == NULL) {
			assets->sprites_defs[i].nb_images = 0;
		}
		else {
			assets->sprites_defs[i].images[0] = tmp;
			assets->sprites_defs[i].nb_images = 1;
		}
		assets->sprites_defs[i].type = types[i];


		sprintf(path, "./data/Images/Defenses/%s/1.png", names[i]);
		tmp = MLV_load_image(path);
		assets->sprites_defs_menu[i] = tmp;
	}
}

void loadChabsolu(Assets* assets) {
	int i;
	char path[MAX_PATH_LEN];
	for (i = 0; i < 3; i++) {
		sprintf(path, "./data/Images/Cats/chabsolu/%d.png", i);
		assets->chabsolu[i] = MLV_load_image(path);
	}
}

void loadFonts(Assets* assets) {
	int i;
	for (i = 0; i < assets->nb_fonts; i++) {
		MLV_free_font(assets->fonts[0]);
	}
	assets->nb_fonts = 3;
	assets->fonts[0] = MLV_load_font("./data/Font/04B_30__.TTF", 30 * window_width / 1344);
	assets->fonts[1] = MLV_load_font("./data/Font/04B_30__.TTF", 15 * window_width / 1344);
	assets->fonts[2] = MLV_load_font("./data/Font/04B_30__.TTF", 60 * window_width / 1344);
}

void loadAnim(Animations* anim) {
	int i = 0;
	char path[MAX_PATH_LEN];
	MLV_Image* tmp;

	while (1) {
		sprintf(path, "./data/Animations/cat_death/%d.png", i);
		tmp = MLV_load_image(path);
		if (tmp == NULL) break;
		anim->cat_death.frames[i] = tmp;
		i++;
	}
	anim->cat_death.nb_frames = i;

	for (i = 0; i < 16; i++) {
		anim->frames[i] = NULL;
	}
}

void loadMenu(Assets* assets) {
	int i;
	char path[MAX_PATH_LEN];

	assets->pattounes = MLV_load_image("./data/Images/Menu/pattounes.png");
	assets->menu = MLV_load_image("./data/Images/Menu/menu.png");
	assets->loading = MLV_load_image("./data/Images/Menu/loading.png");
	assets->bg_menu = MLV_load_image("./data/Images/Menu/bg_menu.png");

	assets->win = MLV_load_image("./data/Images/Menu/win.png");
	assets->lost = MLV_load_image("./data/Images/Menu/lost.png");

	for (i = 0; i < 19; i++) {
		sprintf(path, "./data/Images/Catpedia/%d.png", i + 1);
		assets->catpedia[i] = MLV_load_image(path);
	}
}


/* Affichage */


void showCases(Background bg) {
	int a, b;
	for (a = 0; a < nb_line; a++) {
		for (b = 0; b < nb_column - 1; b++) {
			MLV_draw_rectangle(bg.start_x + b * bg.case_width, bg.start_y + a * bg.path_size, bg.case_width, bg.path_size, MLV_COLOR_BLACK);
		}
	}
}

void displayBackground(Game g, Assets* assets) {
	MLV_draw_image(assets->bg.image, 0, 0);
	MLV_draw_image(assets->bg.shadows, 0, 0);
	MLV_draw_image(assets->chabsolu[g.life], 0, window_height * 55 / 100 - assets->bg.start_x * 1 / 3);
}

void displayLifeBar(Assets* assets, int life, int max_life, int x, int y) {
	int size = assets->size_sprites;
	MLV_draw_filled_rectangle(x, y + size, size, size / 5, MLV_COLOR_BLACK);
	MLV_draw_filled_rectangle(x + size / 20, y + size + size / 20, size - size / 10, size / 5 - size / 10, MLV_COLOR_RED);
	MLV_draw_filled_rectangle(x + size / 20, y + size + size / 20, (size - size / 10) * (max(0, life)) / max_life, size / 5 - size / 10, MLV_COLOR_GREEN);
}

void displayCats(Game g, Assets* assets) {
	Cats* tmp = g.cats;
	int i, x, y;

	while (tmp) {
		if (tmp->position <= nb_column) {
			for (i = 0; i < 11; i++) {
				if (assets->sprites_cats[i].type == tmp->type) {
					posToCoord(assets, tmp->line, tmp->position, &x, &y);
					if (assets->sprites_cats[i].nb_images > 0)
					MLV_draw_image(assets->sprites_cats[i].images[tmp->id_sprite], x, y);
					displayLifeBar(assets, tmp->life, tmp->max_life, x, y);
					break;
				}
			}
		}
		tmp = tmp->next;
	}
}

void displayDefenses(Game g, Assets* assets) {
	Defenses* tmp = g.defenses;
	int i, x, y;

	while (tmp) {
		if (tmp->position <= nb_column) {
			for (i = 0; i < 8; i++) {
				if (assets->sprites_defs[i].type == tmp->type) {
					posToCoord(assets, tmp->line, tmp->position, &x, &y);
					if (assets->sprites_defs[i].nb_images != 0)
					MLV_draw_image(assets->sprites_defs[i].images[0], x, y);
					displayLifeBar(assets, tmp->life, tmp->max_life, x, y);
					break;
				}
			}
		}
		tmp = tmp->next;
	}
}

void displayBuildMenuInGame(Game g, Assets* assets) {
	if (g.money < 10000) {
		MLV_draw_text_box_with_font(window_height * 1 / 200, window_height * 1 / 200, window_width * 12 / 100, window_height * 9 / 100, "%d", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, g.money);
	}
	else {
		MLV_draw_text_box_with_font(window_height * 1 / 200, window_height * 1 / 200, window_width * 12 / 100, window_height * 9 / 100, "%d", assets->fonts[1], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, g.money);
	}
	MLV_draw_image(assets->pattounes, window_height * 1 / 200 + (window_width * 12 / 100) * 85 / 100, window_height * 1 / 200 + (window_height * 9 / 100) * 35 / 100);

	MLV_draw_text_box_with_font(window_height * 3 / 200 + window_width * 12 / 100, window_height * 1 / 200, window_width * 20 / 100, window_height * 9 / 100, "%d", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, g.score);
	MLV_draw_text_box_with_font(window_width - window_height * 23 / 200 - 2 * window_width * 8 / 100, window_height * 1 / 200, window_width * 8 / 100, window_height * 9 / 100, "%d", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, g.level);
	MLV_draw_text_box_with_font(window_width - window_height * 21 / 200 - window_width * 8 / 100, window_height * 1 / 200, window_width * 8 / 100, window_height * 9 / 100, "%d", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, g.turn);
	MLV_draw_text_box_with_font(window_width - window_height * 19 / 200, window_height * 1 / 200, window_height * 9 / 100, window_height * 9 / 100, "II", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
}

void displayBuildMenuGameStart(Game g, Assets* assets, int id_def) {
	int i;
	int prices[8] = {100, 100, 50, 200, 100, 350, 150, 50};

	if (g.money < 10000) {
		MLV_draw_text_box_with_font(window_height * 1 / 200, window_height * 1 / 200, window_width * 12 / 100, window_height * 9 / 100, "%d", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, g.money);
	}
	else {
		MLV_draw_text_box_with_font(window_height * 1 / 200, window_height * 1 / 200, window_width * 12 / 100, window_height * 9 / 100, "%d", assets->fonts[1], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, g.money);
	}
	MLV_draw_image(assets->pattounes, window_height * 1 / 200 + (window_width * 12 / 100) * 85 / 100, window_height * 1 / 200 + (window_height * 9 / 100) * 35 / 100);

	for (i = 0; i < 8; i++) {
		if (id_def == i)
			MLV_draw_text_box_with_font(window_height * 6 / 200 + window_width * 12 / 100 + i * (window_width * 7 / 100), window_height * 1 / 200, window_width * 7 / 100, window_height * 9 / 100, "%d", assets->fonts[1], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_SALMON, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_BOTTOM, prices[i]);
		else
			MLV_draw_text_box_with_font(window_height * 6 / 200 + window_width * 12 / 100 + i * (window_width * 7 / 100), window_height * 1 / 200, window_width * 7 / 100, window_height * 9 / 100, "%d", assets->fonts[1], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_BOTTOM, prices[i]);
		MLV_draw_image(assets->pattounes, window_height * 6 / 200 + window_width * 12 / 100 + i * (window_width * 7 / 100) + (window_width * 7 / 100) * 75 / 100, window_height * 1 / 200 + (window_height * 9 / 100) * 75 / 100);
		if (assets->sprites_defs_menu[i])
			MLV_draw_image(assets->sprites_defs_menu[i], window_height * 6 / 200 + window_width * 12 / 100 + (i * 2 + 1) * (window_width * 7 / 200) - MLV_get_image_width(assets->sprites_defs_menu[i]) / 2, window_height * 1 / 100);
	}
	if (id_def == 8)
		MLV_draw_text_box_with_font(window_height * 6 / 200 + window_width * 12 / 100 + 8 * (window_width * 7 / 100), window_height * 1 / 200, window_width * 7 / 100, window_height * 9 / 100, "Vendre", assets->fonts[1], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_SALMON, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	else
		MLV_draw_text_box_with_font(window_height * 6 / 200 + window_width * 12 / 100 + 8 * (window_width * 7 / 100), window_height * 1 / 200, window_width * 7 / 100, window_height * 9 / 100, "Vendre", assets->fonts[1], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);

	MLV_draw_text_box_with_font(window_width - window_height * 41 / 200 - window_width * 10 / 100, window_height * 1 / 200, window_width * 10 / 100, window_height * 9 / 100, "Vague", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box_with_font(window_width - window_height * 39 / 200, window_height * 1 / 200, window_height * 9 / 100, window_height * 9 / 100, "GO", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box_with_font(window_width - window_height * 19 / 200, window_height * 1 / 200, window_height * 9 / 100, window_height * 9 / 100, "II", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
}

int displayWave(Game g, Assets* assets, int page) {
	Cats* tmp = g.cats;
	int i, x, y;
	int page_buttons = 0;

	while (tmp) {
		if (tmp->turn < page * nb_column) {
			if (page_buttons == 0) page_buttons = 1;
		}
		else if (tmp->turn >= (page + 1) * nb_column) {
			if (page_buttons == 0) page_buttons = 2;
			else if (page_buttons == 1) page_buttons = 3;
		}
		else {
			for (i = 0; i < 10; i++) {

				if (assets->sprites_cats[i].type == tmp->type) {
					posToCoord(assets, tmp->line, tmp->turn - (page * nb_column), &x, &y);
					if (assets->sprites_cats[i].nb_images > 0)
						MLV_draw_image(assets->sprites_cats[i].images[tmp->id_sprite], x, y);
					displayLifeBar(assets, tmp->life, tmp->max_life, x, y);
					break;
				}
			}
		}
		tmp = tmp->next;
	}
	if (page_buttons == 1 || page_buttons == 3) {
		MLV_draw_text_box_with_font(window_height * 1 / 200, window_height * 199 / 200 - window_height * 7 / 100, window_height * 4 / 100, window_height * 7 / 100, "<", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	}
	if (page_buttons == 2 || page_buttons == 3) {
		MLV_draw_text_box_with_font(window_height * 2 / 200 + window_height * 4 / 100, window_height * 199 / 200 - window_height * 7 / 100, window_height * 4 / 100, window_height * 7 / 100, ">", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	}

	return page_buttons;
}

void displayAll(Game g, Assets* assets) {
	displayBackground(g, assets);

	displayDefenses(g, assets);
	displayCats(g, assets);

	displayBuildMenuInGame(g, assets);

	MLV_actualise_window();
}

void displayPauseScreen(Game* g, Assets* assets, int type_background, int wave, int id_def) {
	if (type_background == 2) {
		displayAll(*g, assets);
	} else {
		displayBackground(*g, assets);
		if (type_background == 0) {
			displayDefenses(*g, assets);
			showCases(assets->bg);
		} else if (type_background == 1) {
			displayWave(*g, assets, wave);
		}
		displayBuildMenuGameStart(*g, assets, id_def);
	}
	MLV_draw_filled_rectangle(0, 0, window_width, window_height, MLV_rgba(0, 0, 0, 200));
	MLV_draw_text_box_with_font(window_width * 3 / 10, window_height / 2 - window_height / 8 - window_height / 20, window_width * 2 / 5, window_height / 8, "Reprendre", assets->fonts[2], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box_with_font(window_width * 3 / 10, window_height / 2 + window_height / 20, window_width * 2 / 5, window_height / 8, "Menu", assets->fonts[2], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_COLOR_PINK, MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);

	MLV_actualise_window();
}

void displayLevelChoice(Assets* assets) {
	int i = 1, line, column, size;

	size =  window_width * 6 / 100;

	MLV_draw_image(assets->bg_menu, 0, 0);

	for (line = 0; line < 4; line++) {
		for (column = 0; column < 10; column++) {
			MLV_draw_text_box_with_font(size * 12 / 10 + size * column * 3 / 2, size * 12 / 10 + size * line * 3 / 2, size, size, "%d", assets->fonts[0], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_rgba(190, 130, 220, 255), MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, i);
			i++;
			if (i > NB_LEVELS) break;
		}
		if (i > NB_LEVELS) break;
	}

	MLV_draw_text_box_with_font(window_width * 3 / 10, window_height - window_height / 8 - window_height / 30, window_width * 2 / 5, window_height / 8, "Menu", assets->fonts[2], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_rgba(190, 130, 220, 255), MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_actualise_window();
}

void displayMenu(Assets* assets) {
	MLV_draw_image(assets->menu, 0, 0);
	MLV_draw_text_box_with_font(window_width * 3 / 10, window_height - 4 * (window_height / 8 + window_height / 30), window_width * 2 / 5, window_height / 8, "Classique", assets->fonts[2], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_rgba(190, 130, 220, 255), MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box_with_font(window_width * 3 / 10, window_height - 3 * (window_height / 8 + window_height / 30), window_width * 2 / 5, window_height / 8, "Infini", assets->fonts[2], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_rgba(190, 130, 220, 255), MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box_with_font(window_width * 3 / 10, window_height - 2 * (window_height / 8 + window_height / 30), window_width * 2 / 5, window_height / 8, "Catpedia", assets->fonts[2], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_rgba(190, 130, 220, 255), MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_draw_text_box_with_font(window_width * 3 / 10, window_height - window_height / 8 - window_height / 30, window_width * 2 / 5, window_height / 8, "Quitter", assets->fonts[2], 0, MLV_COLOR_BLACK, MLV_COLOR_BLACK, MLV_rgba(190, 130, 220, 255), MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
	MLV_actualise_window();
}


/* Animations */


void saveDeadCats(Game* g, Assets* assets, Animations* anim) {
	Cats* tmp = g->cats;
	int i, x, y;

	for (i = 0; i < 16; i++) {
		if (anim->frames[i]) {
			MLV_free_image(anim->frames[i]);
			anim->frames[i] = NULL;
		}
	}

	anim->nb_frames = 0;

	while (tmp) {
		if (tmp->position <= nb_column && tmp->life <= 0) {
			if (anim->nb_frames == 0) {
				anim->nb_frames = anim->cat_death.nb_frames;
				for (i = 0; i < anim->nb_frames; i++) {
					anim->frames[i] = MLV_copy_image(assets->bg.image);
				}
			}
			for (i = 0; i < anim->nb_frames; i++) {
				posToCoord(assets, tmp->line, tmp->position, &x, &y);
				MLV_draw_image_on_image(anim->cat_death.frames[i], anim->frames[i], x, y);

			}
		}
		tmp = tmp->next;
	}
}

void playAnim(Game* g, Assets* assets, Animations* anim) {
	int i;
	for (i = 0; i < anim->nb_frames; i++) {
		MLV_draw_image(anim->frames[i], 0, 0);

		MLV_draw_image(assets->bg.shadows, 0, 0);
		MLV_draw_image(assets->chabsolu[g->life], 0, window_height * 55 / 100 - assets->bg.start_x * 1 / 3);

		displayDefenses(*g, assets);
		displayCats(*g, assets);

		displayBuildMenuInGame(*g, assets);

		MLV_actualise_window();
		delay(100);
	}
}


/* Intéractions */


int pauseScreen(Game* g, Assets* assets, int type_background, int wave, int id_def) {
	int x, y;

	while (1) {
		displayPauseScreen(g, assets, type_background, wave, id_def);

		MLV_wait_mouse(&x, &y);

		if (window_width * 3 / 10 <= x && x <= window_width * 3 / 10 + window_width * 2 / 5 && window_height / 2 - window_height / 8 - window_height / 20 <= y && y <= window_height / 2 - window_height / 8 - window_height / 20 + window_height / 8) {
			return 1;
		}
		if (window_width * 3 / 10 <= x && x <= window_width * 3 / 10 + window_width * 2 / 5 && window_height / 2 + window_height / 20 <= y && y <= window_height / 2 + window_height / 20 + window_height / 8) {
			return 0;
		}
	}
}

int buttonsPlaceDefensesGraphic(Game* g, Assets* assets, int x, int y, int* is_wave, int* wave, int page_buttons, int* id_def) {
	char types[8] = {'o', ']', '(', '>', '_', '@', '{', '&'};
	int i, j;

	/* Vagues */
	if (window_width - window_height * 41 / 200 - window_width * 10 / 100 <= x && x <= window_width - window_height * 41 / 200 - window_width * 10 / 100 + window_width * 10 / 100 && window_height * 1 / 200 <= y && y <= window_height * 1 / 200 + window_height * 9 / 100) {
		*is_wave = 1 - *is_wave;
	}

	/* Go */
	else if (window_width - window_height * 39 / 200 <= x && x <= window_width - window_height * 39 / 200 + window_height * 9 / 100 && window_height * 1 / 200 <= y && y <= window_height * 1 / 200 + window_height * 9 / 100) {
		return 1;
	}

	/* Pause */
	else if (window_width - window_height * 19 / 200 <= x && x <= window_width - window_height * 19 / 200 + window_height * 9 / 100 && window_height * 1 / 200 <= y && y <= window_height * 1 / 200 + window_height * 9 / 100) {
		return 2;
	}

	/* Fleches de pages */
	else if (*is_wave) {
		if ((page_buttons == 1 || page_buttons == 3) && window_height * 1 / 200 <= x && x <= window_height * 1 / 200 + window_height * 4 / 100 && window_height * 199 / 200 - window_height * 7 / 100 <= y && y <= window_height * 199 / 200 - window_height * 7 / 100 + window_height * 7 / 100) {
			*wave -= 1;
		}
		else if ((page_buttons == 2 || page_buttons == 3) && window_height * 2 / 200 + window_height * 4 / 100 <= x && x <= window_height * 2 / 200 + window_height * 4 / 100 + window_height * 4 / 100 && window_height * 199 / 200 - window_height * 7 / 100 <= y && y <= window_height * 199 / 200 - window_height * 7 / 100 + window_height * 7 / 100) {
			*wave += 1;
		}
	}

	else {
		/* Changer de construction */
		for (i = 0; i < 9; i++) {
			if (window_height * 6 / 200 + window_width * 12 / 100 + i * (window_width * 7 / 100) <= x && x <= window_height * 6 / 200 + window_width * 12 / 100 + i * (window_width * 7 / 100) + window_width * 7 / 100 && window_height * 1 / 200 <= y && y <= window_height * 1 / 200 + window_height * 9 / 100) {
				*id_def = i;
				break;
			}
		}
		/* Grille */
		if (y >= window_height / 10) {
			for (i = 0; i < nb_line; i++) {
				for (j = 0; j < nb_column - 1; j++) {
					if (assets->bg.start_x + j * assets->bg.case_width <= x && x <= assets->bg.start_x + j * assets->bg.case_width + assets->bg.case_width && assets->bg.start_y + i * assets->bg.path_size <= y && y <= assets->bg.start_y + i * assets->bg.path_size + assets->bg.path_size) {
						if (*id_def == 8) {
							if (existDefense(*g, i + 1, j + 1)) {
								deleteDefenseCoord(g, i + 1, j + 1);
							}
						} else {
							if (!existDefense(*g, i + 1, j + 1)) {
								addDefense(g, i + 1, j + 1, types[*id_def], 1);
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

int placeDefensesGraphic(Game* g, Assets* assets) {
	int x, y, is_wave = 0, wave = 0, page_buttons, id_def = 0, clic_button, is_in_game = 1;
	
	while (is_in_game) {
		displayBackground(*g, assets);

		if (is_wave) {
			page_buttons = displayWave(*g, assets, wave);
		} else {
			displayDefenses(*g, assets);
			showCases(assets->bg);
		}

		displayBuildMenuGameStart(*g, assets, id_def);

		MLV_actualise_window();

		MLV_wait_mouse(&x, &y);

		clic_button = buttonsPlaceDefensesGraphic(g, assets, x, y, &is_wave, &wave, page_buttons, &id_def);

		if (clic_button == 1) {
			/* GO */
			return 1;
		}
		if (clic_button == 2) {
			/* Pause */
			is_in_game = pauseScreen(g, assets, is_wave, wave, id_def);
			if (is_in_game == 0) return 0;
		}
	}
	return 0;
}

void catpedia(Assets* assets) {
	int page = 0, x, y, i;

	while (1) {
		MLV_draw_image(assets->catpedia[page], 0, 0);
		MLV_actualise_window();
		MLV_wait_mouse(&x, &y);

		if (0 <= x && x <= window_width * 150 / 1920 && 0 <= y && y <= window_height * 150 / 1080) {
			return;
		}

		for (i = 0; i < 9; i++) {
			if (window_width * 357 / 1920 + i * window_width * 150 / 1920 <= x && x <= window_width * 357 / 1920 + (i + 1) * window_width * 150 / 1920 && 0 <= y && y <= window_height * 150 / 1080) {
				page = i;
				break;
			}
		}
		for (i = 0; i < 10; i++) {
			if (window_width * 210 / 1920 + i * window_width * 150 / 1920 <= x && x <= window_width * 210 / 1920 + (i + 1) * window_width * 150 / 1920 && window_height * 930 / 1080 <= y && y <= window_height) {
				page = 9 + i;
				break;
			}
		}
	}
}


/* Liberation de mémoire */


void freeSpritesCats(Assets* assets) {
	int i, j;

	for (i = 0; i < 11; i++) {
		for (j = 0; j < assets->sprites_cats[i].nb_images; j++) {
			MLV_free_image(assets->sprites_cats[i].images[j]);
			assets->sprites_cats[i].images[j] = NULL;
		}
	}
}

void freeSpritesDefs(Assets* assets) {
	int i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < assets->sprites_defs[i].nb_images; j++) {
			MLV_free_image(assets->sprites_defs[i].images[j]);
			assets->sprites_defs[i].images[j] = NULL;
		}
	}

	for (i = 0; i < 8; i++) {
		MLV_free_image(assets->sprites_defs_menu[i]);
		assets->sprites_defs_menu[i] = NULL;
	}
}

void freeChabsolu(Assets* assets) {
	int i;

	for (i = 0; i < 3; i++) {
		MLV_free_image(assets->chabsolu[i]);
		assets->chabsolu[i] = NULL;
	}
}

void freeCatDeathAnim(Animations* anim) {
	int i;

	for (i = 0; i < anim->cat_death.nb_frames; i++) {
		MLV_free_image(anim->cat_death.frames[i]);
		anim->cat_death.frames[i] = NULL;
	}

	for (i = 0; i < 16; i++) {
		if (anim->frames[i] != NULL) {
			MLV_free_image(anim->frames[i]);
			anim->frames[i] = NULL;
		}
	}
}

void freeGraphic(Assets* assets, Animations* anim) {
	int i;

	if (assets->chabsolu[0] != NULL) {
		freeChabsolu(assets);
	}
	if (assets->sprites_cats->images[0] != NULL) {
		freeSpritesCats(assets);
	}
	if (assets->sprites_defs->images[0] != NULL) {
		freeSpritesDefs(assets);
	}
	if (anim->cat_death.frames[0] != NULL) {
		freeCatDeathAnim(anim);
	}

	if (assets->bg.image != NULL) {
		MLV_free_image(assets->bg.image);
		assets->bg.image = NULL;

		MLV_free_image(assets->bg.shadows);
		assets->bg.shadows = NULL;
	}

	MLV_free_image(assets->menu);
	assets->menu = NULL;

	MLV_free_image(assets->pattounes);
	assets->pattounes = NULL;

	MLV_free_image(assets->loading);
	assets->loading = NULL;

	MLV_free_image(assets->win);
	assets->win = NULL;

	MLV_free_image(assets->lost);
	assets->lost = NULL;

	for (i = 0; i < 19; i++) {
		if (assets->catpedia[i]) {
			MLV_free_image(assets->catpedia[i]);
			assets->catpedia[i] = NULL;
		}
	}

	for (i = 0; i < assets->nb_fonts; i++) {
		MLV_free_font(assets->fonts[i]);
		assets->fonts[i] = NULL;
	}

	MLV_free_window();
}


/*************************************************************/
/*                  Utilitaires Chattack                     */
/*************************************************************/


int loadLevel(Game* g, char name[]) {
	FILE* file = NULL;
	char file_line[MAX_FILE_LINE_SIZE];
	int turn, line, i;
	char type;

	file = fopen(name, "r");

	if (file == NULL) return 1;

	if (fgets(file_line, MAX_FILE_LINE_SIZE, file)) {
		g->money = atoi(file_line);
	}

	if (fgets(file_line, MAX_FILE_LINE_SIZE, file)) {
		nb_line = atoi(file_line);
	}

	if (fgets(file_line, MAX_FILE_LINE_SIZE, file)) {
		nb_column = atoi(file_line);
	}

	while (fgets(file_line, MAX_FILE_LINE_SIZE, file)) {
		i = 0;

		/* Tours */
		turn = 0;
		while (file_line[i] != ' ') {
			turn = turn *  10 + file_line[i] - 48;
			i++;
		}
		i++;

		/* Lignes */
		line = 0;
		while (file_line[i] != ' ') {
			line = line *  10 + file_line[i] - 48;
			i++;
		}
		i++;

		/* Types */
		type = file_line[i];

		if (addCat(g, turn, line, type) == 1) return 1;
	}

	fclose(file);
	return 0;
}

int youDied(Game g) {
	Cats* tmp = g.cats;
	while (tmp) {
		if (tmp->position <= 0) return 1;
		tmp = tmp->next;
	}
	return 0;
}

int youWin(Game g) {
	if (g.cats == NULL) return 1;
	return 0;
}

void reloadSprites(Assets* assets, Animations* anim) {
	if (assets->chabsolu[0] != NULL) {
		freeChabsolu(assets);
	}
	if (assets->sprites_cats->images[0] != NULL) {
		freeSpritesCats(assets);
	}
	if (assets->sprites_defs->images[0] != NULL) {
		freeSpritesDefs(assets);
	}
	if (anim->cat_death.frames[0] != NULL) {
		freeCatDeathAnim(anim);
	}

	loadChabsolu(assets);
	loadCatsSprites(assets);
	loadDefensesSprites(assets);
	loadAnim(anim);
}

void deleteDead(Game* g, Assets* assets, Animations* anim, int graphic, int is_infinite){
	if (graphic) saveDeadCats(g, assets, anim);
	deleteDeadCats(g, is_infinite);
	deleteDeadDefenses(g);
	if (graphic) playAnim(g, assets, anim);
}

int turnASCII(Game g, int autoASCII) {
	printf("\e[1;1H\e[2J");
	printTurn(g);

	if (youDied(g)) {
		printf("\e[1;1H\e[2J");
		printf("T'es mort !\n");
		return 0;
	}

	if (youWin(g)) {
		printf("\nBravo, vous avez vaincu la vague de chats !\n");
		return 0;
	}

	if (autoASCII) delay(1000);
	else {
		printf("\nAppuyez sur Entree pour passer au tour suivant.\n");
		getchar();
	}

	return 1;
}

int turnGraphic(Game* g, Assets* assets, int is_infinite) {
	int x, y, has_clic;

	displayAll(*g, assets);

	if (youDied(*g)) {
		if (is_infinite && g->life > 0) {
			actionChabsolu(g);
			g->life -= 1;
			return 1;
		} else {
			MLV_draw_image(assets->lost, 0, 0);
			MLV_actualise_window();
			MLV_wait_mouse(&x, &y);
			return 0;
		}
	}

	if (youWin(*g)) {
		if (is_infinite) return 2;
		MLV_draw_image(assets->win, 0, 0);
		MLV_actualise_window();
		MLV_wait_mouse(&x, &y);
		return 0;
	}

	has_clic = MLV_wait_mouse_or_seconds(&x, &y, 1);

	if (has_clic && window_width - window_height * 19 / 200 <= x && x <= window_width - window_height * 19 / 200 + window_height * 9 / 100 && window_height * 1 / 200 <= y && y <= window_height * 1 / 200 + window_height * 9 / 100) {
		return pauseScreen(g, assets, 2, 0, 0);
	}

	return 1;
}

void initGraphic(Game* g, Assets* assets, Animations* anim) {
	loadMenu(assets);
	resizeMenu(assets);

	MLV_draw_image(assets->loading, 0, 0);
	MLV_actualise_window();

	assets->nb_fonts = 0;
	loadFonts(assets);

	assets->chabsolu[0] = NULL;
	assets->sprites_cats->images[0] = NULL;
	assets->sprites_defs->images[0] = NULL;
	anim->cat_death.frames[0] = NULL;

	assets->bg.image = NULL;
}

int gameLevels(Game* g, Assets* assets, Animations* anim, char level[], int graphic, int autoASCII) {
	int is_playing = 1;
	Cats* tmp_cat;
	Defenses* tmp_def;

	loadLevel(g, level);

	g->score = 0;
	g->turn = 0;
	g->life = 2;

	if (!graphic) {
		placeDefensesASCII(g);
		getchar();
	} else {
		MLV_draw_image(assets->loading, 0, 0);
		MLV_actualise_window();
		loadBackground(&(assets->bg), "Plains");
		reloadSprites(assets, anim);
		resizeSprites(assets);
		resizeAnim(assets, anim);
		assignCatSprites(g, assets);
		is_playing = placeDefensesGraphic(g, assets);
	}

	while (is_playing) {
		g->turn++;

		moveIncomingCats(g);

		actionDefenses(g);
		deleteDead(g, assets, anim, graphic, 0);

		actionCats(g);
		deleteDead(g, assets, anim, graphic, 0);

		moveVisibleCats(g);
		deleteDead(g, assets, anim, graphic, 0);

		if (!graphic) {
			is_playing = turnASCII(*g, autoASCII);
		} else {
			is_playing = turnGraphic(g, assets, 0);
		}
		deleteDead(g, assets, anim, graphic, 0);
	}

	tmp_cat = g->cats;
	while (tmp_cat) {
		tmp_cat = deleteFirstCat(g, tmp_cat);
	}

	tmp_def = g->defenses;
	while (tmp_def) {
		tmp_def = deleteFirstDefense(g, tmp_def);
	}

	return 0;
}


/*************************************************************/
/*                       Mode Infini                         */
/*************************************************************/


void waveGenerator(Game* g, int* max_rand) {
	int line, column, num_rand;
	int level = g->level;

	*max_rand += level;

	while (g->cats == NULL) {
		for (column = 1; column <= level * 2; column++) {
			for (line = 1; line <= nb_line; line++) {
				if (rand() % (nb_line * 2) == 0) {
					num_rand = rand() % *max_rand;

					if (num_rand < level)
						addCat(g, column, line, 'C');

					else if (level >= 2 && num_rand < 1 + 2 * (level - 1))
						addCat(g, column, line, 'c');

					else if (level >= 3 && num_rand < 0 + 3 * (level - 1))
						addCat(g, column, line, '#');

					else if (level >= 4 && num_rand < -2 + 4 * (level - 1))
						addCat(g, column, line, '+');

					else if (level >= 5 && num_rand < -5 + 5 * (level - 1))
						addCat(g, column, line, 'x');

					else if (level >= 6 && num_rand < -9 + 6 * (level - 1))
						addCat(g, column, line, '<');

					else if (level >= 7 && num_rand < -14 + 7 * (level - 1))
						addCat(g, column, line, '^');

					else if (level >= 8 && num_rand < -20 + 8 * (level - 1))
						addCat(g, column, line, '.');

					else if (level >= 9 && num_rand < -27 + 9 * (level - 1))
						addCat(g, column, line, 'A');

					else if (level >= 10 && num_rand < -35 + 10 * (level - 1))
						addCat(g, column, line, '*');
				}
			}
		}
	}
}

int gameLevelsInfinite(Game* g, Assets* assets, Animations* anim) {
	int state, is_playing;

	g->turn = 0;

	is_playing = placeDefensesGraphic(g, assets);

	while (is_playing) {
		g->turn++;

		moveIncomingCats(g);

		actionDefenses(g);
		deleteDead(g, assets, anim, 1, 1);

		actionCats(g);
		deleteDead(g, assets, anim, 1, 1);

		moveVisibleCats(g);
		deleteDead(g, assets, anim, 1, 1);

		state = turnGraphic(g, assets, 1);

		deleteDead(g, assets, anim, 1, 1);

		if (state == 2) return 1;
		if (state == 0) return 0;
	}

	return 0;
}

void infiniteMode(Game* g, Assets* assets, Animations* anim) {
	int play_infinite = 1, max_rand = 0;
	Cats* tmp_cat;
	Defenses* tmp_def;

	g->score = 0;
	g->level = 0;
	g->life = 2;

	nb_line = 7;
	nb_column = 24;

	g->money = nb_line * 100;

	MLV_draw_image(assets->loading, 0, 0);
	MLV_actualise_window();
	loadBackground(&(assets->bg), "Plains");
	reloadSprites(assets, anim);
	resizeSprites(assets);
	resizeAnim(assets, anim);

	while (play_infinite) {
		g->level++;
		
		waveGenerator(g, &max_rand);

		assignCatSprites(g, assets);

		play_infinite = gameLevelsInfinite(g, assets, anim);
	}

	tmp_cat = g->cats;
	while (tmp_cat) {
		tmp_cat = deleteFirstCat(g, tmp_cat);
	}

	tmp_def = g->defenses;
	while (tmp_def) {
		tmp_def = deleteFirstDefense(g, tmp_def);
	}
}


/*************************************************************/
/*                           Menus                           */
/*************************************************************/


/* ASCII */


void menuASCII(Game* g) {
	int autoASCII = 1, level = 1;
	char path[MAX_PATH_LEN];

	do {
		printf("Voulez-vous jouer en mode manuel (0) ou en mode automatique (1) ?\n");
		printf("(Le mode manuel demandera d'appuyer sur Entree pour passer les tours alors que le mode automatique passera les tours au bout d'un certain temps)\n ==> ");
		scanf("%d", &autoASCII);
	} while (autoASCII != 0 && autoASCII != 1);

	do {
		printf("A quel niveau souhaitez-vous jouer (nombre entre 1 et %d) ?\n", NB_LEVELS);
		scanf("%d", &level);
	} while (level < 1 || level > NB_LEVELS);

	sprintf(path, "./data/Levels/%d.txt", level);

	gameLevels(g, NULL, NULL, path, 0, autoASCII);
}


/* Graphique */


void levelChoice(Game* g, Assets* assets, Animations* anim) {
	char path[MAX_PATH_LEN];
	int x, y, i = 1, line, column, size;

	while (1) {
		displayLevelChoice(assets);
		MLV_wait_mouse(&x, &y);

		if (window_width * 3 / 10 <= x && x <= window_width * 3 / 10 + window_width * 2 / 5 && window_height - window_height / 8 - window_height / 30 <= y && y <= window_height - window_height / 8 - window_height / 30 + window_height / 8) {
			return;
		}

		size =  window_width * 6 / 100;

		for (line = 0; line < 4; line++) {
			for (column = 0; column < 10; column++) {
				if (size * 12 / 10 + size * column * 3 / 2 <= x && x <= size * 12 / 10 + size * column * 3 / 2 + size && size * 12 / 10 + size * line * 3 / 2 <= y && y <= size * 12 / 10 + size * line * 3 / 2 + size) {
					sprintf(path, "./data/Levels/%d.txt", i);
					g->level = i;
					gameLevels(g, assets, anim, path, 1, 1);
					return;
				}
				i++;
				if (i > NB_LEVELS) break;
			}
			if (i > NB_LEVELS) break;
		}
	}
}

void menuGraphic(Game* g, Assets* assets, Animations* anim) {
	int x, y, is_full_screen = 0;

	while (1) {
		displayMenu(assets);
		MLV_wait_mouse(&x, &y);

		/* Classique */
		if (window_width * 3 / 10 <= x && x <= window_width * 3 / 10 + window_width * 2 / 5 && window_height - 4 * (window_height / 8 + window_height / 30) <= y && y <= window_height - 4 * (window_height / 8 + window_height / 30) + window_height / 8) {
			levelChoice(g, assets, anim);
		}

		/* Infini */
		else if (window_width * 3 / 10 <= x && x <= window_width * 3 / 10 + window_width * 2 / 5 && window_height - 4 * (window_height / 8 + window_height / 30) <= y && y <= window_height - 3 * (window_height / 8 + window_height / 30) + window_height / 8) {
			infiniteMode(g, assets, anim);
		}

		/* Catpedia */
		else if (window_width * 3 / 10 <= x && x <= window_width * 3 / 10 + window_width * 2 / 5 && window_height - 4 * (window_height / 8 + window_height / 30) <= y && y <= window_height - 2 * (window_height / 8 + window_height / 30) + window_height / 8) {
			catpedia(assets);
		}
		
		/* Quitter */
		else if (window_width * 3 / 10 <= x && x <= window_width * 3 / 10 + window_width * 2 / 5 && window_height - window_height / 8 - window_height / 30 <= y && y <= window_height - window_height / 8 - window_height / 30 + window_height / 8) {
			return;
		}

		/* Plein écran */
		else if (window_width * 95 / 100 <= x && x <= window_width && 0 <= y && y <= window_height * 8 / 100) {
			freeGraphic(assets, anim);
			is_full_screen = switchFullScreen(is_full_screen);
			initGraphic(g, assets, anim);
		}
	}
}


/*************************************************************/
/*                            Main                           */
/*************************************************************/


int main(int argc, char const* argv[]) {
	Game game;
	int graphic = 1;

	srand(time(NULL));

	game.cats = NULL;
	game.defenses = NULL;

	do {
		printf("Voulez-vous jouer en mode ASCII (0) ou en mode graphique (1) ?\n ==> ");
		scanf("%d", &graphic);
	} while (graphic != 0 && graphic != 1);

	if (!graphic) {
		menuASCII(&game);
	} else {
		Assets assets;
		Animations animations;
		createWindow();
		initGraphic(&game, &assets, &animations);
		menuGraphic(&game, &assets, &animations);
		freeGraphic(&assets, &animations);
	}

	return 0;
}
