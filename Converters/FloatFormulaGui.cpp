#include <math.h>
#include <cmath>
#include "./FloatFormulaGui.h"
#include "./my_type_convert.h"

using namespace std;

REGISTER_GUI_PLUGIN( FloatFormulaGui, L"SE Float Function GUI" );

FloatFormulaGui::FloatFormulaGui( IMpUnknown* host ) : MpGuiBase(host)
{
	// initialise pins.
	valueIn.initialize( this, 0, static_cast<MpGuiBaseMemberPtr>(&FloatFormulaGui::onSetValueIn) );
	valueOut.initialize( this, 1, static_cast<MpGuiBaseMemberPtr>(&FloatFormulaGui::onSetValueOut) );
	FormulaA.initialize( this, 2, static_cast<MpGuiBaseMemberPtr>(&FloatFormulaGui::onSetFormulaA) );
	FormulaB.initialize( this, 3, static_cast<MpGuiBaseMemberPtr>(&FloatFormulaGui::onSetFormulaB) );

	formulaA_ascii = "0";
	formulaB_ascii = "0";
}

// handle pin updates.
void FloatFormulaGui::onSetValueIn()
{
	double B;
	double A = valueIn;
	int flags = 0;
	ee.SetValue( "a", &A );
	ee.Evaluate( formulaB_ascii.c_str(), &B, &flags );

	if(!std::isfinite(B) )
	{
		valueOut = 0.0f;
	}

	valueOut = (float) B;
}

void FloatFormulaGui::onSetValueOut()
{
	double A;
	double B = valueOut;
	int flags = 0;
	ee.SetValue( "b", &B );
	ee.Evaluate( formulaA_ascii.c_str(), &A, &flags );

	if( !std::isfinite(A) )
	{
		valueIn = 0.0f;
	}

	valueIn = (float) A;
}

void FloatFormulaGui::onSetFormulaA()
{
	formulaA_ascii = UnicodeToAscii( FormulaA );
	if( formulaA_ascii.empty() )
	{
		formulaA_ascii = "0";
	}

	onSetValueOut();
}

void FloatFormulaGui::onSetFormulaB()
{
	formulaB_ascii = UnicodeToAscii( FormulaB );
	if( formulaB_ascii.empty() )
	{
		formulaB_ascii = "0";
	}
	onSetValueIn();
}


