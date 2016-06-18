/*
 *  This file is part of tils.
 *
 *  tils is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  tils is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with tils.  If not, see <http://www.gnu.org/licenses/>
 */

/**
 * @file src/lib/queue.c
 *
 * @brief Queue data structure implementation
 *
 * The queue works by maintaining 3 internal queues. At any time, 2 are in
 * use. One is being added to by the producer, and the other is being removed
 * from by the consumer. These queues are called Q_P and Q_C respectively, and
 * stand for the number labeling each buffer.
 *
 * If the Q_C is empty, we increment Q_C
 *
 * Q_C := Q_C + 1 % 3. 
 *
 * Now if Q_C != Q_P, then nothing special happens. If Q_C == Q_P, we need to
 * increment Q_P atomically. The trick is that insertions and removals from 
 * each queue are atomic as well, as in this scenario it is possible that an
 * insertion and a removal are happening concurrently, so it is critical that
 * the same piece of data is not inserted and removed at the same time.
 *
 * @author Lars Wander 
 */

