// Created by Crowbar 0.56

$modelname "custom_props/thing_that_spins_correctly.mdl"

$staticprop

$bodygroup "thing_that_spins_correctly"
{
	studio "thing_that_spins_correctly.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition 0 0 0

$cdmaterials "models\props\mcgravel\"

$cbox 0 0 0 0 0 0

$bbox -1.936 -14.193 -5.806 1.936 14.193 5.806

$definebone "static_prop" "" 0 0 0 0 0 0 0 0 0 0 0 0


$sequence "idle" {
	"thing_that_spins_correctly_anims\idle.smd"
	activity "ACT_IDLE" -1
	fadein 0.2
	fadeout 0.2
	fps 1
}
