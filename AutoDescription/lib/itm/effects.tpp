DEFINE_PATCH_FUNCTION ~get_description_effect~ RET description sort BEGIN
	SET opcode_n = opcode
	SET durationAdded = 0
	SET saveAdded = 0

	LPM ~block_to_vars~

	SPRINT description ~~
	SPRINT opcode_target ~~

	PATCH_IF abilityType == AbilityType_Equipped BEGIN
		SPRINT opcode_target ~_self~
		SPRINT theTarget   @102472 // ~le porteur~
		SPRINT ofTheTarget @101086 // ~du porteur~
	END
	ELSE BEGIN
		// TODO: Si abilityType == AbilityType_Combat ou AbilityType_Charge, préciser qu'il faut ajouter "au porteur|du porteur|le porteur ou à la cible|de la cible|la cible"
		// Serait ajouté dans une variable qu'il suffira d'utiliser
		// Pas très i18n friendly par contre, car cela se base sur la construction des phrases en Français... mais bon, pas grave !
		PATCH_IF target == TARGET_FX_self BEGIN
			SPRINT opcode_target ~_self~
			SPRINT theTarget   @102472 // ~le porteur~
			SPRINT ofTheTarget @101086 // ~du porteur~
		END
		ELSE PATCH_IF target == TARGET_FX_preset OR target == TARGET_FX_none OR target == TARGET_FX_everyone_except_self BEGIN
			SPRINT opcode_target ~_target~
			SPRINT theTarget   @102471 // ~la cible~
			SPRINT ofTheTarget @101085 // ~de la cible~
		END
		ELSE PATCH_IF target == TARGET_FX_party BEGIN
			SPRINT opcode_target ~_party~
			SPRINT theTarget   @102473 // ~le groupe~
			SPRINT ofTheTarget @101088 // ~du groupe~
		END
	END

	PATCH_IF probability == 100 BEGIN
		LPM ~opcode%opcode_target%_%opcode_n%~
		LPM ~set_opcode_sort~
	END
	ELSE BEGIN
		SET probability += 1
		LPM ~opcode%opcode_target%_probability_%opcode_n%~
		LPM ~set_opcode_sort~
		PATCH_IF NOT ~%description%~ STRING_EQUAL ~~ BEGIN
			LPF ~percent_value~ INT_VAR value = EVAL ~%probability%~ RET probability = value END
			SPRINT description @101125 // ~%probability% de chance %description%~
		END
	END

	LPM ~add_duration~
	LPM ~add_save~
END

DEFINE_PATCH_MACRO ~set_opcode_sort~ BEGIN
	PATCH_IF opcode == 219 BEGIN
		SET opcode = opcode_n
	END

	PATCH_IF VARIABLE_IS_SET $sort_opcodes(~%opcode%~) BEGIN
		SET sort = $sort_opcodes(~%opcode%~) + ((100 - probability) * 10000)
	END
	ELSE BEGIN
		SET sort = 400 + ((100 - probability) * 10000)
	END
END

DEFINE_PATCH_FUNCTION ~get_description_effect2~ RET description saveAdded durationAdded BEGIN
	READ_SHORT EFF2_opcode opcode
	READ_BYTE  EFF2_power power
	READ_LONG  EFF2_parameter1 parameter1
	READ_LONG  EFF2_parameter2 parameter2
	READ_LONG  EFF2_duration duration
	READ_BYTE  EFF2_probability1 probability1
	READ_BYTE  EFF2_probability2 probability2
	READ_ASCII EFF2_resource resref
	READ_LONG  EFF2_dice_thrown diceCount
	READ_LONG  EFF2_dice_sides diceSides
	READ_LONG  EFF2_save_type saveType
	READ_LONG  EFF2_save_bonus saveBonus
	READ_LONG  EFF2_parameter3 parameter3
	READ_LONG  EFF2_parameter4 parameter4

	SET parentProbability = probability
	SET probability = probability1 - probability2
	SPRINT description ~~

	PATCH_IF NOT VARIABLE_IS_SET $ignored_opcodes(~%opcode%~) BEGIN
		PATCH_IF probability == 100 AND (~%macro%~ STRING_MATCHES_REGEXP ~_probability$~) == 0 BEGIN
			PATCH_FAIL "%item%: %SOURCE_FILE%: probabilite differentes du 177 et de l'effet pointe."
		END
		PATCH_IF probability < 100 OR parentProbability < 100 BEGIN
			LPM ~%macro%probability_%opcode%~
			PATCH_IF NOT ~%description%~ STRING_EQUAL ~~ AND parentProbability == 100 BEGIN
				LPF ~percent_value~ INT_VAR value = EVAL ~%probability%~ RET probability = value END
				SPRINT description @101125 // ~%probability% de chance %description%~
			END
		END
		ELSE BEGIN
			LPM ~%macro%%opcode%~
		END
	END

	LPM ~add_duration~
	LPM ~add_save~
END

DEFINE_PATCH_MACRO ~add_duration~ BEGIN
	PATCH_IF durationAdded == 0 AND NOT ~%description%~ STRING_EQUAL ~~ BEGIN
		LPF ~get_duration_value~ INT_VAR duration RET duration = value END

		PATCH_IF NOT ~%duration%~ STRING_EQUAL ~~ BEGIN
			SPRINT description ~%description% %duration%~
			SET durationAdded = 1
		END
	END
END

DEFINE_PATCH_MACRO ~add_save~ BEGIN
	PATCH_IF saveAdded == 0 AND NOT ~%description%~ STRING_EQUAL ~~ AND saveType != 0 BEGIN
		SPRINT saveTypeStr ~~
        PATCH_IF (saveType BAND FLAG_SAVINGTHROW_spell) == FLAG_SAVINGTHROW_spell BEGIN
            SPRINT saveTypeStr @102033 // ~contre les sorts~
        END
        PATCH_IF (saveType BAND FLAG_SAVINGTHROW_breath) == FLAG_SAVINGTHROW_breath BEGIN
            PATCH_IF NOT ~%saveTypeStr%~ STRING_EQUAL ~~ BEGIN
				SPRINT strOr @100004 // ~ou~
                SPRINT saveTypeStr ~%saveTypeStr% %strOr% ~
            END
            SPRINT saveTypeStr @102032 // ~contre les souffles~
        END
        PATCH_IF (saveType BAND FLAG_SAVINGTHROW_death) == FLAG_SAVINGTHROW_death BEGIN
            PATCH_IF NOT ~%saveTypeStr%~ STRING_EQUAL ~~ BEGIN
				SPRINT strOr @100004 // ~ou~
                SPRINT saveTypeStr ~%saveTypeStr% %strOr% ~
            END
            SPRINT saveTypeStr @102029 // ~contre la paralysie, la mort et les poisons~
        END
        PATCH_IF (saveType BAND FLAG_SAVINGTHROW_wand) == FLAG_SAVINGTHROW_wand BEGIN
            PATCH_IF NOT ~%saveTypeStr%~ STRING_EQUAL ~~ BEGIN
				SPRINT strOr @100004 // ~ou~
                SPRINT saveTypeStr ~%saveTypeStr% %strOr% ~
            END
            SPRINT saveTypeStr @102030 // ~contre les baguettes, les sceptres et les bâtons~
        END
        PATCH_IF (saveType BAND FLAG_SAVINGTHROW_polymorph) == FLAG_SAVINGTHROW_polymorph BEGIN
            PATCH_IF NOT ~%saveTypeStr%~ STRING_EQUAL ~~ BEGIN
				SPRINT strOr @100004 // ~ou~
                SPRINT saveTypeStr ~%saveTypeStr% %strOr% ~
            END
            SPRINT saveTypeStr @102031 // ~contre la pétrification et la métamorphose~
        END

		// TODO: Toujours pour éviter ??

        PATCH_IF saveBonus != 0 BEGIN
			LPF ~signed_value~ INT_VAR value = EVAL ~%saveBonus%~ RET saveBonus = value END
			SPRINT saveStr @102122 // ~jet de sauvegarde à %saveBonus% %saveTypeStr% pour éviter~
        END
        ELSE BEGIN
			SPRINT saveStr @102121 // ~jet de sauvegarde %saveTypeStr% pour éviter~
        END

		SPRINT description ~%description% (%saveStr%)~
		SET saveAdded = 1
	END
END

DEFINE_PATCH_MACRO ~block_to_vars~ BEGIN
	READ_SHORT (blockOffset) opcode
	READ_BYTE  (blockOffset + EFF_target) target
	READ_BYTE  (blockOffset + EFF_power) power
	READ_LONG  (blockOffset + EFF_parameter1) parameter1
	READ_LONG  (blockOffset + EFF_parameter2) parameter2
	READ_BYTE  (blockOffset + EFF_timing_mode) timingMode
	READ_BYTE  (blockOffset + EFF_resistance) resistance
	READ_LONG  (blockOffset + EFF_duration) duration
	READ_BYTE  (blockOffset + EFF_probability1) probability1
	READ_BYTE  (blockOffset + EFF_probability2) probability2
	READ_ASCII (blockOffset + EFF_resref_key) resref
	READ_LONG  (blockOffset + EFF_dice_count) diceCount
	READ_LONG  (blockOffset + EFF_dice_sides) diceSides
	READ_LONG  (blockOffset + EFF_save_type) saveType
	READ_LONG  (blockOffset + EFF_save_bonus) saveBonus
	READ_LONG  (blockOffset + 0x2c) special

	SET probability = probability1 - probability2
END

DEFINE_PATCH_MACRO ~abilities_groups_to_vars~ BEGIN
	SPRINT opcode "%data_0%"
	SPRINT target "%data_1%"
	SPRINT power "%data_2%"
	SPRINT parameter1 "%data_3%"
	SPRINT parameter2 "%data_4%"
	SPRINT timingMode "%data_5%"
	SPRINT resistance "%data_6%"
	SPRINT duration "%data_7%"
	SPRINT probability1 "%data_8%"
	SPRINT probability2 "%data_9%"
	SPRINT resref "%data_10%"
	SPRINT diceCount "%data_11%"
	SPRINT diceSides "%data_12%"
	SPRINT saveType "%data_13%"
	SPRINT saveBonus "%data_14%"
	SPRINT special "%data_15%"
END


DEFINE_PATCH_FUNCTION ~get_damage_value~ INT_VAR diceCount = 0 diceSides = 0 damageAmount = 0 RET damage BEGIN
	SPRINT ~damage~ ~~

	PATCH_IF diceCount > 0 AND diceSides > 0 BEGIN
		PATCH_IF diceSides == 1 BEGIN
			SET damageAmount += diceCount
		END
		ELSE BEGIN
			SPRINT ~damage~ @10014 // ~%diceCount%d%diceSides%~
		END
	END
	PATCH_IF damageAmount > 0 BEGIN
		LPF ~signed_value~ INT_VAR value = EVAL ~%damageAmount%~ RET damageAmount = value END
		PATCH_IF ~%damage%~ STRING_EQUAL ~~ BEGIN
			SPRINT ~damage~ ~%damageAmount%~
		END
		ELSE BEGIN
			SPRINT ~damage~ ~%damage% %damageAmount%~
		END
	END
END

DEFINE_PATCH_FUNCTION ~get_duration_value~ INT_VAR duration = 0 RET value BEGIN
	SPRINT value ~~
	PATCH_IF timingMode == TIMING_permanent BEGIN
		// TODO: A gérer mieux que ça
		// SPRINT value @100046 // ~de manière permanente~
	END
	ELSE PATCH_IF timingMode != TIMING_while_equipped AND duration > 0 BEGIN
        LPF ~get_str_duration~ INT_VAR duration RET strDuration END

		PATCH_IF timingMode == TIMING_duration BEGIN
			SPRINT value @100039 // ~pendant %duration%~
		END
		ELSE PATCH_IF timingMode == TIMING_delayed BEGIN
			SPRINT value @100038 // ~après %strDuration%~
		END
		ELSE BEGIN
			PATCH_PRINT "%SOURCE_FILE%: timing %timingMode%"
		END
    END
END

DEFINE_PATCH_FUNCTION ~get_str_duration~ INT_VAR duration = 0 RET strDuration BEGIN
    SET total = duration
    SET turns = duration / 60
    SET duration = duration MODULO 60
    SET rounds = duration / 6
    SET seconds = duration MODULO 6

	SPRINT strDuration ~~

    PATCH_IF seconds > 0 OR (turns > 0 AND rounds > 0) BEGIN
        SET seconds = total
        PATCH_IF seconds == 1 BEGIN
            SPRINT strDuration @100040 // ~1 seconde~
        END
        ELSE BEGIN
            SPRINT strDuration @100041 // ~%seconds% secondes~
        END
    END
    ELSE PATCH_IF turns > 0 BEGIN
        PATCH_IF turns == 1 BEGIN
            SPRINT strDuration @100042 // ~1 tour~
        END
        ELSE BEGIN
            SPRINT strDuration @100043 // ~%turns% tours~
        END
    END
    ELSE PATCH_IF rounds > 0 BEGIN
        PATCH_IF rounds == 1 BEGIN
            SPRINT strDuration @100044 // ~1 round~
        END
        ELSE BEGIN
            SPRINT strDuration @100045 // ~%round% rounds~
        END
    END
END
