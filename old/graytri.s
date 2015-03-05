| Assembly Source File
| Created 2/16/2015; 7:28:42 PM

.data

.globl fill_span_black
.globl fill_span_dark
.globl fill_span_light
.globl fill_span_white

.globl asm_gray_tri

| d0 est l'abscisse de la première extrémité de la ligne x0
| d1 est l'abscisse de la deuxième extrémité de la ligne x1
| a0 est l'adresse du début de ligne dans la mémoire video (dark plane)
| Attention ! La routine détruit a2

fill_span_black:
	cmp.w %d0,%d1 | Si x1 < x0
	bgt no_exg
	exg %d0,%d1 | Echanger x0 et x1
no_exg:
	tst.w %d0
	bge x1_in_scr
	clr.w %d0	| si x0 est négatif, le mettre à 0 (clipping à la limite gauche de l'écran)
	tst.w %d1	| et si x1 aussi est négatif
	bge x1_in_scr
	rts
x1_in_scr:
	move.w lcd_w, %d2
	cmp.w %d2, %d1
	blt x2_in_scr	| si d1 est inférieur à lcd_w (bout de la ligne dans l'écran) brancher
	move.w %d2,%d1  | sinon mettre sa valeur à lcd_w-1 (clipping à la limite droite de l'écran)
	add.w #-1,%d1	|
	cmp.w %d2,%d0  | si d0 est aussi plus grand ou égal que lcd_w
	blt x2_in_scr
	rts
x2_in_scr:
move.l %a2,-(%sp)
	move.w	%d3,-(%a7) | On sauvegarde d3
	move.w	%d0,%d3
	lsr.w	#4,%d3
	add.w	%d3,%d3
	adda.w	%d3,%a0
	lea (3840,%a0),%a1	| charger a1 qui sera le pointeur dans le deuxième plane (light plane) avec a0+LCD_SIZE

        lsl.w	#3,%d3
        addi.w	#16,%d3 | d3=8(x1/8 & 0x1E)+16
        move.w	%d1,%d2 | d2=x2
        andi.w	#15,%d0
        add.w	%d0,%d0
        lea	DEBUT(%pc),%a2
        move.w	(%a2,%d0.w),%d0 | d0=DEBUT[x1 & 15]
        andi.w	#15,%d1
        add.w	%d1,%d1
	lea	FIN(%pc),%a2
        move.w	(%a2,%d1.w),%d1 | d1=FIN[x2 & 15]
        cmp.w	%d3,%d2 | x2-8(x1/8 & 0x1E)+16
        blt.s	hsmall
        sub.w	%d3,%d2 | d2=x2-x
        move.w	(%a7)+,%d3	| Rétablir d3
        or.w	%d0,(%a0)+
        or.w	%d0,(%a1)+	| Ecriture dnas les deux planes
        moveq.l	#0xFFFFFFFF,%d0
        subi.w	#32,%d2
        blt.s	hend
hloop:
        move.l	%d0,(%a0)+
	move.l	%d0,(%a1)+
        subi.w	#32,%d2
        bge.s	hloop
hend:
        cmpi.w	#-16,%d2
        blt.s	end_not_on_grid
        move.w	%d0,(%a0)+ | La dernière partie de la ligne à afficher est un mot
        move.w	%d0,(%a1)+
end_not_on_grid:
        or.w	%d1,(%a0)  | La dernière partie de la ligne à afficher est moins d'un mot
        or.w	%d1,(%a1)
move.l (%sp)+,%a2
        rts
hsmall:	| Largeur de ligne inférieure à 16
        move.w	(%a7)+,%d3
        and.w	%d0,%d1
        or.w	%d1,(%a0)
        or.w	%d1,(%a1)	| Ecriture dans les deux planes
move.l (%sp)+,%a2
        rts


fill_span_dark:
	cmp.w %d0,%d1 | Si x1 < x0
	bgt no_exg2
	exg %d0,%d1 | Echanger x0 et x1
no_exg2:
	tst.w %d0
	bge x1_in_scr2
	clr.w %d0	| si x0 est négatif, le mettre à 0 (clipping à la limite gauche de l'écran)
	tst.w %d1	| et si x1 aussi est négatif
	bge x1_in_scr2
	rts
x1_in_scr2:
	move.w lcd_w, %d2
	cmp.w %d2, %d1
	blt x2_in_scr2	| si d1 est inférieur à lcd_w (bout de la ligne dans l'écran) brancher
	move.w %d2,%d1  | sinon mettre sa valeur à lcd_w-1 (clipping à la limite droite de l'écran)
	add.w #-1,%d1	|
	cmp.w %d2,%d0  | si d0 est aussi plus grand ou égal que lcd_w
	blt x2_in_scr2
	rts
x2_in_scr2:
move.l %a2,-(%sp)
	move.w	%d3,-(%a7) | On sauvegarde d3
	move.w	%d0,%d3
	lsr.w	#4,%d3
	add.w	%d3,%d3
	adda.w	%d3,%a0
	lea (3840,%a0),%a1	| charger a1 qui sera le pointeur dans le deuxième plane (light plane) avec a0+LCD_SIZE

        lsl.w	#3,%d3
        addi.w	#16,%d3 | d3=8(x1/8 & 0x1E)+16
        move.w	%d1,%d2 | d2=x2
        andi.w	#15,%d0
        add.w	%d0,%d0
        lea	DEBUT(%pc),%a2
        move.w	(%a2,%d0.w),%d0 | d0=DEBUT[x1 & 15]
        andi.w	#15,%d1
        add.w	%d1,%d1
	lea	FIN(%pc),%a2
        move.w	(%a2,%d1.w),%d1 | d1=FIN[x2 & 15]
        cmp.w	%d3,%d2 | x2-8(x1/8 & 0x1E)+16
        blt.s	hsmall2
        sub.w	%d3,%d2 | d2=x2-x
        move.w	(%a7)+,%d3	| Rétablir d3
        or.w	%d0,(%a0)+
        not.w	%d0
        and.w	%d0,(%a1)+	| Ecriture dans les deux planes
        moveq.l	#0xFFFFFFFF,%d0
        subi.w	#32,%d2
        blt.s	hend2
hloop2:
        move.l	%d0,(%a0)+
        not.l	%d0
	move.l	%d0,(%a1)+
	not.l	%d0	| Rétablit %d0 = 0xFFFFFFFF
        subi.w	#32,%d2
        bge.s	hloop2
hend2:
        cmpi.w	#-16,%d2
        blt.s	end_not_on_grid2
        move.w	%d0,(%a0)+ | La dernière partie de la ligne à afficher est un mot
        not.w	%d0
        move.w	%d0,(%a1)+
end_not_on_grid2:
        or.w	%d1,(%a0)  | La dernière partie de la ligne à afficher est moins d'un mot
        not.w	%d1
        and.w	%d1,(%a1)
move.l (%sp)+,%a2
        rts
hsmall2:	| Largeur de ligne inférieure à 16
        move.w	(%a7)+,%d3
        and.w	%d0,%d1
        or.w	%d1,(%a0)
        not.w	%d1
        and.w	%d1,(%a1)	| Ecriture dans les deux planes
move.l (%sp)+,%a2
        rts


fill_span_light:
	cmp.w %d0,%d1 | Si x1 < x0
	bgt no_exg3
	exg %d0,%d1 | Echanger x0 et x1
no_exg3:
	tst.w %d0
	bge x1_in_scr3
	clr.w %d0	| si x0 est négatif, le mettre à 0 (clipping à la limite gauche de l'écran)
	tst.w %d1	| et si x1 aussi est négatif
	bge x1_in_scr3
	rts
x1_in_scr3:
	move.w lcd_w, %d2
	cmp.w %d2, %d1
	blt x2_in_scr3	| si d1 est inférieur à lcd_w (bout de la ligne dans l'écran) brancher
	move.w %d2,%d1  | sinon mettre sa valeur à lcd_w-1 (clipping à la limite droite de l'écran)
	add.w #-1,%d1	|
	cmp.w %d2,%d0  | si d0 est aussi plus grand ou égal que lcd_w
	blt x2_in_scr3
	rts
x2_in_scr3:
move.l %a2,-(%sp)
	move.w	%d3,-(%a7) | On sauvegarde d3
	move.w	%d0,%d3
	lsr.w	#4,%d3
	add.w	%d3,%d3
	adda.w	%d3,%a0
	lea (3840,%a0),%a1	| charger a1 qui sera le pointeur dans le deuxième plane (light plane) avec a0+LCD_SIZE

        lsl.w	#3,%d3
        addi.w	#16,%d3 | d3=8(x1/8 & 0x1E)+16
        move.w	%d1,%d2 | d2=x2
        andi.w	#15,%d0
        add.w	%d0,%d0
        lea	DEBUT(%pc),%a2
        move.w	(%a2,%d0.w),%d0 | d0=DEBUT[x1 & 15]
        andi.w	#15,%d1
        add.w	%d1,%d1
	lea	FIN(%pc),%a2
        move.w	(%a2,%d1.w),%d1 | d1=FIN[x2 & 15]
        cmp.w	%d3,%d2 | x2-8(x1/8 & 0x1E)+16
        blt.s	hsmall3
        sub.w	%d3,%d2 | d2=x2-x
        move.w	(%a7)+,%d3	| Rétablir d3
        or.w	%d0,(%a1)+
        not.w	%d0
        and.w	%d0,(%a0)+	| Ecriture dans les deux planes
        moveq.l	#0xFFFFFFFF,%d0
        subi.w	#32,%d2
        blt.s	hend3
hloop3:
        move.l	%d0,(%a1)+
        not.l	%d0
	move.l	%d0,(%a0)+
	not.l	%d0	| Rétablit %d0 = 0xFFFFFFFF
        subi.w	#32,%d2
        bge.s	hloop3
hend3:
        cmpi.w	#-16,%d2
        blt.s	end_not_on_grid3
        move.w	%d0,(%a1)+ | La dernière partie de la ligne à afficher est un mot
        not.w	%d0
        move.w	%d0,(%a0)+
end_not_on_grid3:
        or.w	%d1,(%a1)  | La dernière partie de la ligne à afficher est moins d'un mot
        not.w	%d1
        and.w	%d1,(%a0)
move.l (%sp)+,%a2
        rts
hsmall3:	| Largeur de ligne inférieure à 16
        move.w	(%a7)+,%d3
        and.w	%d0,%d1
        or.w	%d1,(%a1)
        not.w	%d1
        and.w	%d1,(%a0)	| Ecriture dans les deux planes
move.l (%sp)+,%a2
        rts


fill_span_white:
	cmp.w %d0,%d1 | Si x1 < x0
	bgt no_exg4
	exg %d0,%d1 | Echanger x0 et x1
no_exg4:
	tst.w %d0
	bge x1_in_scr4
	clr.w %d0	| si x0 est négatif, le mettre à 0 (clipping à la limite gauche de l'écran)
	tst.w %d1	| et si x1 aussi est négatif
	bge x1_in_scr4
	rts
x1_in_scr4:
	move.w lcd_w, %d2
	cmp.w %d2, %d1
	blt x2_in_scr4	| si d1 est inférieur à lcd_w (bout de la ligne dans l'écran) brancher
	move.w %d2,%d1  | sinon mettre sa valeur à lcd_w-1 (clipping à la limite droite de l'écran)
	add.w #-1,%d1	|
	cmp.w %d2,%d0  | si d0 est aussi plus grand ou égal que lcd_w
	blt x2_in_scr4
	rts
x2_in_scr4:
move.l %a2,-(%sp)
	move.w	%d3,-(%a7) | On sauvegarde d3
	move.w	%d0,%d3
	lsr.w	#4,%d3
	add.w	%d3,%d3
	adda.w	%d3,%a0
	lea (3840,%a0),%a1	| charger a1 qui sera le pointeur dans le deuxième plane (light plane) avec a0+LCD_SIZE

        lsl.w	#3,%d3
        addi.w	#16,%d3 | d3=8(x1/8 & 0x1E)+16
        move.w	%d1,%d2 | d2=x2
        andi.w	#15,%d0
        add.w	%d0,%d0
        lea	DEBUT(%pc),%a2
        move.w	(%a2,%d0.w),%d0 | d0=DEBUT[x1 & 15]
        andi.w	#15,%d1
        add.w	%d1,%d1
	lea	FIN(%pc),%a2
        move.w	(%a2,%d1.w),%d1 | d1=FIN[x2 & 15]
        cmp.w	%d3,%d2 | x2-8(x1/8 & 0x1E)+16
        blt.s	hsmall4
        sub.w	%d3,%d2 | d2=x2-x
        move.w	(%a7)+,%d3	| Rétablir d3
        not.w	%d0
        and.w	%d0,(%a1)+
        and.w	%d0,(%a0)+	| Ecriture dans les deux planes
        moveq.l	#0,%d0
        subi.w	#32,%d2
        blt.s	hend4
hloop4:
        move.l	%d0,(%a1)+
        move.l	%d0,(%a0)+
	subi.w	#32,%d2
        bge.s	hloop4
hend4:
        cmpi.w	#-16,%d2
        blt.s	end_not_on_grid4
        move.w	%d0,(%a1)+ | La dernière partie de la ligne à afficher est un mot
        move.w	%d0,(%a0)+
end_not_on_grid4:
        not.w	%d1
        and.w	%d1,(%a1)  | La dernière partie de la ligne à afficher est moins d'un mot
        and.w	%d1,(%a0)
move.l (%sp)+,%a2
        rts
hsmall4:	| Largeur de ligne inférieure à 16
        move.w	(%a7)+,%d3
        and.w	%d0,%d1
        not.w	%d1
        and.w	%d1,(%a1)
        and.w	%d1,(%a0)	| Ecriture dans les deux planes
move.l (%sp)+,%a2
        rts

|--------------------------------------------------------
| Dessine un triangle en niveaux de gris
| Paramètres : 3 points x1=d0 y1=d1
|			x2=d2 y2=d3
|			x3=d4 y3=d5
| et la couleur : d6
|--------------------------------------------------------
asm_gray_tri:
	movem.l %d7/%a2-%a6,-(%sp)

	link.w %a6,#-22			| Allouer un stackframe comme espace de stockage transitoire (4 mots + 3 mots longs)

	cmpi.b #3,%d6			| couleur = BLACK ?
	bne not_black			| si non
	move.l #fill_span_black,%a5	| charge l'adresse de fill_span_black dans a5
	bra color_loaded
not_black:
	cmpi.b #2,%d6			| couleur = D_GRAY ?
	bne not_dark			| si non
	move.l #fill_span_dark,%a5	| charge l'adresse de fill_span_dark dans a5
	bra color_loaded
not_dark:
	cmpi.b #1,%d6			| couleur = L_GRAY ?
	bne not_light			| si non
	move.l #fill_span_light,%a5	| charge l'adresse de fill_span_light dans a5
	bra color_loaded
not_light:
	tst.b %d6			| couleur = WHITE ?
	bne tri_return			| si la couleur à ce stade n'est pas 0, alors elle n'est pas comprise en 0 et 3, retour
	move.l #fill_span_white,%a5	| charge l'adresse de fill_span_white dans a5
color_loaded:
| on peut maintenant utiliser d6 qui ne sert à présent plus à rien

	cmp.w %d0,%d2
	bne not_degenerated
	cmp.w %d2,%d4
	bne not_degenerated
	cmp.w %d1,%d3
	bne not_degenerated
	cmp.w %d3,%d5
	beq tri_return	| si x1=x2=x3 et y1=y2=y3 return
not_degenerated:
	cmp.w %d1,%d3
	ble tri_no_exg1
	exg %d0,%d2
	exg %d1,%d3	| if (y2>=y1) echanger (x1,y1) avec (x2,y2)
tri_no_exg1:
	cmp.w %d1,%d5
	ble tri_no_exg2
	exg %d0,%d4
	exg %d1,%d5 	| if (y3>=y1) echanger (x1,y1) avec (x3,y3)
tri_no_exg2:
	cmp.w %d3,%d5
	ble tri_no_exg3
	exg %d2,%d4
	exg %d3,%d5	| if (y3>=y2) echanger (x2,y2) avec (x3,y3)
tri_no_exg3:
| maintenant les points sont classés de telle manière que y1 >= y2 >= y3

	move.w %d0, -16(%a6)
	move.w %d1, -18(%a6)
	move.w %d2, -20(%a6)
	move.w %d3, -22(%a6)	| sauver les coordonnées des trois points sur le stackframe

|calcul de dx/dy pour le couple de points 1-3
	sub.w %d4,%d0		| d0 contient ensuite dx = x1-x3
	swap %d0		| multiplier par 2^16
	move.w %d1,%d6		| charger y1 dans d6
	sub.w %d5,%d6		| d6 contient dy = y1-y3
	tst.w %d6
	beq dy_zero1		| division ssi d6 != 0
	ext.l %d6		| extension de signe sur d6
	move.l %d6,-(%sp)
	move.l %d0,-(%sp)	| passer les arguments en paramètres, d'abord le deuxième dx, puis (dy<<16)
	jbsr __divsi3		| division sur long, après d0 vaut le résultat de la division
	addq.l #8,%sp		| La fonction ne désempile pas les pramètres, il faut donc le faire manuellement (sans récupérer les données)
	move.l %d0,-4(%a6)	| stocker dx/dy dans le stackframe alloué au début
dy_zero_return1:
|calcul de dx/dy pour le couple de points 2-3
	clr.l %d0		| Effacer la partie de poids fort de d0
	move.w -20(%a6),%d0	| charger d0 avec x2 (d2 à été touché donc on récupère x2 sur le stackframe)
	sub.w %d4,%d0		| d0 contient ensuite dx = x2-x3
	swap %d0		| multiplier par 2^16
	move.w %d3,%d6		| charger y2 dans d6
	sub.w %d5,%d6		| d6 contient dy = y2-y3
	tst.w %d6
	beq dy_zero2		| division ssi d6 != 0
	ext.l %d6		| extension de signe sur d6
	move.l %d6,-(%sp)
	move.l %d0,-(%sp)	| passer les arguments en paramètres, d'abord le deuxième dx, puis (dy<<16)
	jbsr __divsi3		| division sur long, après d0 vaut le résultat de la division
	addq.l #8,%sp		| La fonction ne désempile pas les pramètres, il faut donc le faire manuellement (sans récupérer les données)
	move.l %d0,-8(%a6)	| stocker dx/dy dans le stackframe alloué au début
dy_zero_return2:
	clr.l %d0		| Effacer la partie de poids fort de d0
	move.w -16(%a6),%d0	| charger d0 avec x1 (d0 à été touché donc on récupère x1 sur le stackframe)
	sub.w -20(%a6),%d0	| d0 contient ensuite dx = x1-x2 (d2 à été touché donc on récupère x2 sur le stackframe)
	swap %d0		| multiplier par 2^16
	move.w -18(%a6),%d6	| charger y1 dans d6
	sub.w %d3,%d6		| d6 contient dy = y1-y2 (d3 n'a pas été touché)
	tst.w %d6
	beq dy_zero3		| division ssi d6 != 0
	ext.l %d6		| extension de signe sur d6
	move.l %d6,-(%sp)
	move.l %d0,-(%sp)	| passer les arguments en paramètres, d'abord le deuxième dx, puis (dy<<16)
	jbsr __divsi3		| division sur long, après d0 vaut le résultat de la division
	addq.l #8,%sp		| La fonction ne désempile pas les pramètres, il faut donc le faire manuellement (sans récupérer les données)
	move.l %d0,-12(%a6)	| stocker dx/dy dans le stackframe alloué au début
dy_zero_return3:

| La suite utilise 5 registres de données et un registre d'adresse
| d0 et d1 seront les registres chargés juste avant d'appeller une routine fill_span
| d3 sera l'abscisse du début de la ligne courante (xa) et d4 l'abscisse de l'autre bout (xb)
| d5 sera l'ordonnée courante (y)
| a3 sera l'adresse du début de la ligne courante
| d6 = lcd_h
	move.w lcd_h,%d6

	clr.l %d3	| effacer d3 (surtout sa partie de poids fort)
	move.w %d4,%d3	| charger d3 (xa) avec x3
	swap %d3	| décaler de 16 bits
	move.l %d3,%d4	| au début xa = xb

	cmpi.w #999, -8(%a6)	| si m2 = 999, c'est à dire les points 1 et 2 ont la même ordonnée (le triangle à un côté horizontal au dessus), il faut faire xb = x2
	bne top_summit
	clr.l %d4		| effacer d4 (surtout sa partie de poids fort)
	move.w -20(%a6), %d4	| charger d4 avec x2
	swap %d4		| décaler de 16 bits
top_summit:

| maintenant que les pentes sont calculées et que les points de départ initialisés, les boucles qui appelle les fill_span comme il faut
| Calcul de addr = Vscreen1+(y3<<5)-(y3<<1) = Vscreen1 + 30*y3
	move.l Vscreen1, %a3
	move.w %d5,%d2		| charger d2 avec y3
	asl.w #5, %d2
	add.w %d2,%a3		| Ajouter y3<<5 à a3
	move.w %d5,%d2
	asl.w #1, %d2
	sub.w %d2,%a3		| Retrancher y3>>1 à a3
| Initialisation des variables de boucle : il n'y à rien à faire, d5 = y vaut déja y3 la valeur initiale voulue (on démarre en haut du triangle)
| Boucle en elle même
loop1:
| Test des conditions :
	cmp.w -22(%a6),%d5	| Comparer y à y2
	bge loop1_break		| Si y>=y2, terminer la boucle
| Corps du code de la boucle, c'est à dire appel de fill_span
	tst.w %d5
	blt y_out1
	cmp.w %d6,%d5
	bge y_out1
	move.l %d3,%d0		| Charger xa et xb dans les registres qui serviront d'entrée à fill_span
	move.l %d4,%d1
	swap %d0
	swap %d1		| slide de 16 vers la droite
	ext.l %d0		| au cas ou la partie de poids faible n'était pas vide
	ext.l %d1
	move.l %a3,%a0		| charger l'adresse du début de ligne pour fill_span
	jsr (%a5)		| Appeller la routine fill_span choisie suivant la couleur au début de la routine (d0-d2 seront détruits)
y_out1:
| Evolution des paramètres
	add.l -4(%a6),%d3	| xa += m1
	add.l -8(%a6),%d4	| xb += m2
	add.l #30,%a3		| addr += 30 (passage au début de ligne suivante)
	add.w #1,%d5		| incrémenter y (le numéro de ligne)
	bra loop1
loop1_break:

| Initialisation des variables de la 2e boucle : il n'y à rien à faire, d5 = y vaut déja y2
| Boucle 2 en elle même
loop2:
| Test des conditions :
	cmp.w -18(%a6),%d5	| Comparer y à y1
	bge loop2_break		| Si y>=y1, terminer la boucle
	cmp.w (lcd_h),%d5
	bge loop2_break
| Corps du code de la boucle, c'est à dire appel de fill_span
	tst.w %d5
	blt y_out2
	cmp.w %d6,%d5
	bge y_out2
	move.l %d3,%d0		| Charger xa et xb dans les registres qui serviront d'entrée à fill_span
	move.l %d4,%d1
	swap %d0
	swap %d1		| slides de 16 vers la droite
	ext.l %d0		| au cas ou la partie de poids faible n'était pas vide
	ext.l %d1
	move.l %a3,%a0		| charger l'adresse du début de ligne pour fill_span
	jsr (%a5)		| Appeller la routine fill_span choisie suivant la couleur au début de la routine (d0-d2 seront détruits)
y_out2:
| Evolution des paramètres
	add.l -4(%a6),%d3	| xa += m1
	add.l -12(%a6),%d4	| xb += m3
	add.l #30,%a3		| addr += 30 (passage au début de ligne suivante)
	add.w #1,%d5		| incrémenter y (le numéro de ligne)
	bra loop2
loop2_break:
| Ca y est c'est fini, registres détruits d0-d6/a0-a1 (a0 et a1 détruits pas fill_span)
tri_return:
	unlk %a6		| désallouer le stackframe
	movem.l (%sp)+, %d7/%a2-%a6	| retablir les registres sauvegardés sur la pile
	rts			| retour à la routine appellante
|-------------------------------|
	.even
dy_zero1:
	move.w #999, -4(%a6)	| sinon m1 = 999
	bra dy_zero_return1
dy_zero2:
	move.w #999, -8(%a6)	| sinon m2 = 999
	bra dy_zero_return2
dy_zero3:
	move.w #999, -12(%a6)	| sinon m3 = 999
	bra dy_zero_return3

|---------------------------------------------------------


	.even
DEBUT:
	dc.w 0xffff
	dc.w 0x7fff
	dc.w 0x3fff
	dc.w 0x1fff
	dc.w 0x0fff
	dc.w 0x07ff
	dc.w 0x03ff
	dc.w 0x01ff
	dc.w 0x00ff
	dc.w 0x007f
	dc.w 0x003f
	dc.w 0x001f
	dc.w 0x000f
	dc.w 0x0007
	dc.w 0x0003
	dc.w 0x0001

FIN:
	dc.w 0x8000
	dc.w 0xc000
	dc.w 0xe000
	dc.w 0xf000
	dc.w 0xf800
	dc.w 0xfc00
	dc.w 0xfe00
	dc.w 0xff00
	dc.w 0xff80
	dc.w 0xffc0
	dc.w 0xffe0
	dc.w 0xfff0
	dc.w 0xfff8
	dc.w 0xfffc
	dc.w 0xfffe
	dc.w 0xffff

