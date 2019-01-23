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

typedef struct OldLink
{
    struct OldLink* prev;
    struct OldLink* next;
} X_Link;

static inline void x_link_insert_after(OldLink* linkToInsert, OldLink* after)
{
    linkToInsert->next = after->next;
    linkToInsert->prev = after;
    after->next = linkToInsert;
    linkToInsert->next->prev = linkToInsert;
}

static inline void x_link_insert_before(OldLink* linkToInsert, OldLink* before)
{
    linkToInsert->next = before;
    linkToInsert->prev = before->prev;
    linkToInsert->prev->next = linkToInsert;
    linkToInsert->next->prev = linkToInsert;
}

static inline void x_link_clear(OldLink* link)
{
    link->next = NULL;
    link->prev = NULL;
}

static inline void x_link_init(OldLink* head, OldLink* tail)
{
    head->next = tail;
    head->prev = NULL;
    
    tail->next = NULL;
    tail->prev = head;
}

static inline void x_link_init_self(OldLink* link)
{
    link->next = link;
    link->prev = link;
}

static inline void x_link_unlink(OldLink* link)
{
    link->prev->next = link->next;
    link->next->prev = link->prev;
    
    x_link_init_self(link);
}

