/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/

#ifndef LOGICEXPRESSION_H
#define LOGICEXPRESSION_H
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMutexLocker>
#include <QtCore/QVector>

class ExpressionWrapper;
class Expression
{
public:
        ~Expression();
        Expression();
        Expression(const QString &aExpression) { m_expr=nullptr; m_execList=nullptr; setExpression(aExpression); }
        Expression(const QString &expression, ExpressionWrapper *wrapper) { m_expr=nullptr; m_execList=nullptr; setExpression(expression); mModelObject = wrapper;  }
        // intialization
        void setExpression(const QString &aExpression); ///< set expression
        void setAndParse(const QString &expr); ///< set expression and parse instantly
        void setModelObject(ExpressionWrapper *wrapper) { mModelObject = wrapper; }
        const QString &expression() const { return m_expression; }
        void  parse(ExpressionWrapper *wrapper=nullptr); ///< force a parsing of the expression

        /// call linearize() to 'linarize' an expression, i.e. approximate the function by linear interpolation.
        void linearize(const double low_value, const double high_value, const int steps=1000);
        /// lineraize2d works with two variables
        void linearize2d(const double low_x, const double high_x, const double low_y, const double high_y, const int stepsx=50, const int stepsy=50);

        /// global switch for linerization. If set to false, subsequent calls to linearize are ignored.
        static void setLinearizationEnabled(const bool enable) {mLinearizationAllowed = enable; }
        // calculations
        double execute(double *varlist=nullptr, ExpressionWrapper *object=nullptr) const; ///< calculate formula and return result. variable values need to be set using "setVar()"
        bool executeBool(double *varlist=nullptr, ExpressionWrapper *object=nullptr) const { return execute(varlist, object) != 0.; }
        double executeLocked() { QMutexLocker m(&m_execMutex); return execute();  } ///< thread safe version
        /** calculate formula. the first two variables are assigned the values Val1 and Val2. This function is for convenience.
           the return is the result of the calculation.
           e.g.: x+3*y --> Val1->x, Val2->y
           forceExecution: do not apply linearization */
        double calculate(const double Val1=0., const double Val2=0., const bool forceExecution=false) const;
        bool calculateBool(const double Val1=0., const double Val2=0., const bool forceExecution=false) const { return calculate(Val1, Val2, forceExecution) != 0.; }
        /// calculate formula with object
        ///
        double calculate(ExpressionWrapper &object, const double variable_value1=0., const double variable_value2=0.) const;
        double calculateBool(ExpressionWrapper &object, const double variable_value1=0., const double variable_value2=0.) const { return calculate(object,variable_value1, variable_value2)!=0.; }

        //variables
        /// set the value of the variable named "Var". Note: using addVar to obtain a pointer may be more efficient for multiple executions.
        void  setVar(const QString& Var, double Value);
        /// adds variable "VarName" and returns a double pointer to the variable. Use *ptr to set the value (before calling execute())
        double *addVar(const QString& VarName);
        /// retrieve again the value pointer of a variable.
        double *  getVarAdress(const QString& VarName);


        bool isConstExpression() const { return m_constExpression; } ///< returns true if current expression is a constant.
        bool isEmpty() const { return m_empty; } ///< returns true if expression is empty
        const QString &lastError() const { return m_errorMsg; }
        /** strict property: if true, variables must be named before execution.
          When strict=true, all variables in the expression must be added by setVar or addVar.
          if false, variable values are assigned depending on occurence. strict is false by default for calls to "calculate()".
        */
        bool isStrict() { return m_strict;}
        void setStrict(bool str) { m_strict=str; }
        void setCatchExceptions(bool docatch=true) { m_catchExceptions = docatch; }
        void   setExternalVarSpace(const QStringList& ExternSpaceNames, double* ExternSpace);
        void enableIncSum();
        // other maintenance
        static void addConstant(const QString const_name, const double const_value);
private:
        enum ETokType {etNumber, etOperator, etVariable, etFunction, etLogical, etCompare, etStop, etUnknown, etDelimeter};
        enum EValueClasses {evcBHD, evcHoehe, evcAlter};
        struct ExtExecListItem {
            ETokType Type;
            double  Value;
            int     Index;
        };
        enum EDatatype {edtInfo, edtNumber, edtString, edtObject, edtVoid, edtObjVar, edtReference, edtObjectReference};
        bool m_catchExceptions;
        QString m_errorMsg;

        bool m_parsed;
        mutable bool m_strict;
        bool m_empty; // empty expression
        bool m_constExpression;
        QString m_tokString;
        QString m_expression;
        Expression::ExtExecListItem *m_execList;
        int m_execListSize; // size of buffer
        int m_execIndex;
        double m_varSpace[10];
        QStringList m_varList;
        QStringList m_externVarNames;
        double *m_externVarSpace;
        Expression::ETokType m_state;
        Expression::ETokType m_lastState;
        char *m_pos;
        char *m_expr;
        QString m_token;
        QString m_prepStr;
        int   m_tokCount;
        Expression::ETokType  next_token();
        void  atom();
        void  parse_levelL0();
        void  parse_levelL1();
        void  parse_level0();
        void  parse_level1();
        void  parse_level2();
        void  parse_level3();
        void  parse_level4();
        int  getFuncIndex(const QString& functionName);
        int  getVarIndex(const QString& variableName);
        inline double getModelVar(const int varIdx, ExpressionWrapper *object=nullptr) const ;

        // link to external model variable
        ExpressionWrapper *mModelObject;

        double getExternVar(const int Index) const;
        // inc-sum
        mutable double m_incSumVar;
        bool   m_incSumEnabled;
        double  udfPolygon(double Value, double* Stack, int ArgCount) const; ///< special function polygon()
        double  udfInList(double value, double* stack, int argCount) const; ///< special function in()
        double udfSigmoid(double Value, double sType, double p1, double p2) const; ///< special function sigmoid()
        double udfRandom(int type, double p1, double p2) const; ///< user defined function rnd() (normal distribution does not work now!)

        void checkBuffer(int Index);
        QMutex m_execMutex;
        // linearization
        inline double linearizedValue(const double x) const;
        inline double linearizedValue2d(const double x, const double y) const;
        int mLinearizeMode;
        QVector<double> mLinearized;
        double mLinearLow, mLinearHigh;
        double mLinearStep;
        double mLinearLowY, mLinearHighY;
        double mLinearStepY;
        int mLinearStepCountY;
        static bool mLinearizationAllowed;
};

#endif // LOGICEXPRESSION_H
