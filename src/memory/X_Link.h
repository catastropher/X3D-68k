// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#define x_link_to_struct(link_, type_) ((type*)(link_))

typedef struct X_Link
{
    struct X_Link* prev;
    struct X_Link* next;
} X_Link;

static inline void x_link_insert_after(X_Link* linkToInsert, X_Link* after)
{
    linkToInsert->next = after->next;
    linkToInsert->prev = after;
    after->next = linkToInsert;
    linkToInsert->next->prev = linkToInsert;
}

static inline void x_link_insert_before(X_Link* linkToInsert, X_Link* before)
{
    linkToInsert->next = before;
    linkToInsert->prev = before->prev;
    linkToInsert->prev->next = linkToInsert;
    linkToInsert->next->prev = linkToInsert;
}

static inline void x_link_clear(X_Link* link)
{
    link->next = NULL;
    link->prev = NULL;
}

static inline void x_link_init(X_Link* head, X_Link* tail)
{
    head->next = tail;
    head->prev = NULL;
    
    tail->next = NULL;
    tail->prev = head;
}

static inline void x_link_init_self(X_Link* link)
{
    link->next = link;
    link->prev = link;
}

static inline void x_link_unlink(X_Link* link)
{
    link->prev->next = link->next;
    link->next->prev = link->prev;
    
    x_link_init_self(link);
}

