/*  File         : trace_interception.pl
    Purpose      : predicate to help locating sources while debugging

    pqConsole    : interfacing SWI-Prolog and Qt

    Author       : Carlo Capelli
    E-mail       : cc.carlo.cap@gmail.com
    Copyright (C): 2013,2014,2015,2016

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

:- module(trace_interception, [goal_source_position/5]).

%%  prolog_trace_interception(+Port, +Frame, +Choice, -Action)
%
%   see http://www.swi-prolog.org/pldoc/doc_for?object=prolog_trace_interception/4
%
user:prolog_trace_interception(Port, Frame, Choice, Action) :-
    current_prolog_flag(pq_tracer, F),
    writeln(prolog_trace_interception(F)),
    F=true,
writeln(ok),
    pq_trace_interception(Port, Frame, Choice, Action).

%%  goal_source_position(+Port, +Frame, -Clause, -File, -Position) is det
%
%   access internal frame/clause information to get the
%   source characters position
%
goal_source_position(Port, Frame, Clause, File, A-Z) :-
 pq_trace(1:Port),
    prolog_frame_attribute(Frame, hidden, false),
 pq_trace(2:Frame),
    prolog_frame_attribute(Frame, parent, Parent),
 pq_trace(3:Clause),
    prolog_frame_attribute(Frame, pc, Pc),
 pq_trace(4:File),
    prolog_frame_attribute(Parent, clause, Clause),
 pq_trace(5:(A-Z)),
    clause_info(Clause, File, TermPos, _VarOffsets),
 pq_trace(6),
    locate_vm(Clause, 0, Pc, Pc1, VM),
 pq_trace(7:locate_vm(Clause, 0, Pc, Pc1, VM)),
    '$clause_term_position'(Clause, Pc1, TermPos1),
 pq_trace(8:'$clause_term_position'(Clause, Pc1, TermPos1)),
    ( VM = i_depart(_) -> append(TermPos2, [_], TermPos1) ; TermPos2 = TermPos1 ),
 pq_trace(9:(TermPos2, TermPos)),
    range(TermPos2, TermPos, A, Z),
 pq_trace(10:(range(TermPos2, TermPos, A, Z))).

locate_vm(Clause, X, Pc, Pc1, VM) :-
    '$fetch_vm'(Clause, X, Y, T),
    (   X < Pc
    ->  locate_vm(Clause, Y, Pc, Pc1, VM)
    ;   Pc1 = X, VM = T
    ).

range([], Pos, A, Z) :-
    arg(1, Pos, A),
    arg(2, Pos, Z).
range([H|T], term_position(_, _, _, _, PosL), A, Z) :-
    nth1(H, PosL, Pos),
    range(T, Pos, A, Z).
