#ifndef _SGDQN_H___
#define _SGDQN_H___

/*
   SVM with Quasi-Newton stochastic gradient
   Copyright (C) 2009- Antoine Bordes

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA

   Shogun adjustments (w) 2011 Siddharth Kherada
*/

#include "lib/common.h"
#include "classifier/LinearClassifier.h"
#include "features/DotFeatures.h"
#include "features/Labels.h"

namespace shogun
{
/** @brief class SGDQN */
class CSGDQN : public CLinearClassifier
{
	public:
		/** default constructor  */
		CSGDQN();

		/** constructor
		 *
		 * @param C constant C
		 */
		CSGDQN(float64_t C);

		/** constructor
		 *
		 * @param C constant C
		 * @param traindat training features
		 * @param trainlab labels for training features
		 */
		CSGDQN(
			float64_t C, CDotFeatures* traindat,
			CLabels* trainlab);

		virtual ~CSGDQN();

		/** get classifier type
		 *
		 * @return classifier type SVMSGDQN
		 */
		virtual inline EClassifierType get_classifier_type() { return CT_SGDQN; }

		/** train classifier
		 *
		 * @param data training data (parameter can be avoided if distance or
		 * kernel-based classifiers are used and distance/kernels are
		 * initialized with train data)
		 *
		 * @return whether training was successful
		 */
		virtual bool train(CFeatures* data=NULL);

		/** set C
		 *
		 * @param c_neg new C constant for negatively labeled examples
		 * @param c_pos new C constant for positively labeled examples
		 *
		 */
		inline void set_C(float64_t c_neg, float64_t c_pos) { C1=c_neg; C2=c_pos; }

		/** get C1
		 *
		 * @return C1
		 */
		inline float64_t get_C1() { return C1; }

		/** get C2
		 *
		 * @return C2
		 */
		inline float64_t get_C2() { return C2; }

		/** set epochs
		 *
		 * @param e new number of training epochs
		 */
		inline void set_epochs(int32_t e) { epochs=e; }

		/** get epochs
		 *
		 * @return the number of training epochs
		 */
		inline int32_t get_epochs() { return epochs; }

		/**computing diagonal scaling matrix B as ratio*/
		void compute_ratio(float64_t* W,float64_t* W_1,float64_t* B,float64_t* dst,int32_t dim,float64_t lambda,float64_t loss);

		/** combine and clip scaling matrix B */
		void combine_and_clip(float64_t* Bc,float64_t* B,int32_t dim,float64_t c1,float64_t c2,float64_t v1,float64_t v2);

		/** @return object name */
		inline virtual const char* get_name() const { return "SGDQN"; }

	protected:
		/** calibrate */
		void calibrate();

	private:
		void init();

	private:
		float64_t t;
		float64_t C1;
		float64_t C2;
		int32_t epochs;
		int32_t skip;
		int32_t count;
};
}
#endif
