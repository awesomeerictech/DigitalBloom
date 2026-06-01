// ericdata.js
.pragma library


/*
  Validate the input credential to upload
  Return values are..
  0 - Success
  1 - credentials are empty
  2 -  does not match
*/

function validateReg(param1,param2,param3,param4,param5)
{
    var ret

    if(!(param1 === "" || param2 === "" || param3 === "" || param4 === "" || param5 === "" ) && (param4 === param5) )
    {
        ret = 0
        return ret
    }

    else if(!(param1 === "" || param2 === "" || param3 === "" || param4 === "" || param5 === "" ) && (param4 !== param5))
    {
        ret = 2
    }
    else
    {
        ret = 1
    }

    return ret

}

function validatedat(param1) {

    var ret
    if(!(param1 === "" ) )
    {
        ret = 0
        return ret
    }

    else
    {
        ret = 1
    }
    return ret


}

function validatecrit(param1) {

    var ret
    if((param1 === "" ) || (param1 === "undefined" ) || (param1 === "NaN" ) )
    {
        ret = 1
        return ret
    }

    else
    {
        ret = 0
    }
    return ret


}

function validatesmth(param1,param2,param3)

{

    var ret
    if(!(param1 === "" ) && (param2 === param3) )
    {
        ret = 0
        return ret
    }

    else
    {
        ret = 1
    }
    return ret

}



function validatefive(param1,param2,param3,param4,param5)
{
    var ret
    if(param1 === "" || param2 === "" || param3 === "" || param4 === "" || param5 === ""  )
    {
        ret = 1
        return ret
    }

    else
    {
        ret = 0
    }
    return ret
}

function validatetwo(param1,param2)
{
    var ret
    if(param1 === "" || param2 === ""  )
    {
        ret = 1
        return ret
    }

    else
    {
        ret = 0
    }
    return ret
}

function validateone(param1)
{
    var ret

    if(param1 === "" )
    {
        ret = 1
        return ret
    }

    else
    {
        ret = 0
    }
    return ret
}


function validatefour(param1,param2,param3,param4)
{
    var ret
    if(param1 === "" || param2 === "" || param3 === "" || param4 === "" )
    {
        ret = 1
        return ret
    }

    else
    {
        ret = 0
    }
    return ret
}


function validateseven(param1,param2,param3,param4,param5,param6,param7)
{
    var ret
    if(param1 === "" || param2 === "" || param3 === "" || param4 === "" || param5 === "" || param6 === "" || param7 === "" )
    {
        ret = 1
        return ret
    }

    else
    {
        ret = 0
    }
    return ret
}




