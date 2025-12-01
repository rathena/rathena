"""
Test suite for Moral Alignment System.

Tests cover:
- Alignment vector representation
- Alignment to vector conversion
- Action preference calculations
- Dialogue tone mapping
- Compatibility calculations
- Action morality checks
"""

import pytest

from agents.moral_alignment import (
    MoralAlignment,
    AlignmentVector,
    AlignmentInfluence
)


# ============================================================================
# ALIGNMENT VECTOR TESTS
# ============================================================================

@pytest.mark.unit
def test_alignment_vector_creation():
    """Test alignment vector creation and clamping."""
    # Normal values
    vec = AlignmentVector(law_chaos=0.5, good_evil=0.8)
    assert vec.law_chaos == 0.5
    assert vec.good_evil == 0.8
    
    # Values beyond bounds should be clamped
    vec_clamped = AlignmentVector(law_chaos=2.0, good_evil=-2.0)
    assert vec_clamped.law_chaos == 1.0  # Clamped to max
    assert vec_clamped.good_evil == -1.0  # Clamped to min


@pytest.mark.unit
def test_alignment_vector_bounds():
    """Test alignment vector value bounds."""
    vec = AlignmentVector(law_chaos=1.5, good_evil=-1.5)
    
    # Should clamp to -1.0 to 1.0 range
    assert -1.0 <= vec.law_chaos <= 1.0
    assert -1.0 <= vec.good_evil <= 1.0


# ============================================================================
# ALIGNMENT MAPPING TESTS
# ============================================================================

@pytest.mark.unit
def test_get_vector_for_all_alignments():
    """Test vector retrieval for all alignment types."""
    # Lawful Good - high law, high good
    lg = AlignmentInfluence.get_vector(MoralAlignment.LAWFUL_GOOD)
    assert lg.law_chaos == 1.0
    assert lg.good_evil == 1.0
    
    # Chaotic Evil - low law, low good
    ce = AlignmentInfluence.get_vector(MoralAlignment.CHAOTIC_EVIL)
    assert ce.law_chaos == -1.0
    assert ce.good_evil == -1.0
    
    # True Neutral - balanced
    tn = AlignmentInfluence.get_vector(MoralAlignment.TRUE_NEUTRAL)
    assert tn.law_chaos == 0.0
    assert tn.good_evil == 0.0


@pytest.mark.unit
def test_from_vector_conversion():
    """Test converting vector back to alignment."""
    # Exact matches
    lg_vec = AlignmentVector(1.0, 1.0)
    alignment = AlignmentInfluence.from_vector(lg_vec)
    assert alignment == MoralAlignment.LAWFUL_GOOD
    
    # Close matches should snap to nearest
    near_lg = AlignmentVector(0.9, 0.95)
    alignment = AlignmentInfluence.from_vector(near_lg)
    assert alignment == MoralAlignment.LAWFUL_GOOD
    
    # Neutral
    neutral_vec = AlignmentVector(0.0, 0.0)
    alignment = AlignmentInfluence.from_vector(neutral_vec)
    assert alignment == MoralAlignment.TRUE_NEUTRAL


@pytest.mark.unit
def test_from_vector_finds_closest():
    """Test that from_vector finds the closest alignment."""
    # Slightly lawful, slightly good
    vec = AlignmentVector(0.3, 0.2)
    alignment = AlignmentInfluence.from_vector(vec)
    
    # Should be one of the neutral-ish alignments
    assert alignment in [
        MoralAlignment.LAWFUL_NEUTRAL,
        MoralAlignment.NEUTRAL_GOOD,
        MoralAlignment.TRUE_NEUTRAL,
        MoralAlignment.LAWFUL_GOOD
    ]


# ============================================================================
# ACTION PREFERENCE TESTS
# ============================================================================

@pytest.mark.unit
def test_action_preference_lawful_good():
    """Test action preferences for Lawful Good alignment."""
    alignment = MoralAlignment.LAWFUL_GOOD
    
    # Should prefer helping and following rules
    assert AlignmentInfluence.get_action_preference(alignment, "help_others") > 0.7
    assert AlignmentInfluence.get_action_preference(alignment, "follow_rules") > 0.7
    assert AlignmentInfluence.get_action_preference(alignment, "honor_duty") > 0.7
    
    # Should not prefer harming or breaking rules
    assert AlignmentInfluence.get_action_preference(alignment, "harm_others") < 0.3
    assert AlignmentInfluence.get_action_preference(alignment, "break_rules") < 0.3


@pytest.mark.unit
def test_action_preference_chaotic_evil():
    """Test action preferences for Chaotic Evil alignment."""
    alignment = MoralAlignment.CHAOTIC_EVIL
    
    # Should prefer chaos and selfishness
    assert AlignmentInfluence.get_action_preference(alignment, "break_rules") > 0.7
    assert AlignmentInfluence.get_action_preference(alignment, "selfish_act") > 0.7
    assert AlignmentInfluence.get_action_preference(alignment, "embrace_chaos") > 0.7
    
    # Should not prefer helping or following rules
    assert AlignmentInfluence.get_action_preference(alignment, "help_others") < 0.3
    assert AlignmentInfluence.get_action_preference(alignment, "follow_rules") < 0.3


@pytest.mark.unit
def test_action_preference_true_neutral():
    """Test action preferences for True Neutral alignment."""
    alignment = MoralAlignment.TRUE_NEUTRAL
    
    # All preferences should be around 0.5 (neutral)
    for action in ["help_others", "follow_rules", "break_rules"]:
        pref = AlignmentInfluence.get_action_preference(alignment, action)
        assert 0.3 < pref < 0.7  # Reasonably neutral


@pytest.mark.unit
def test_action_preference_unknown_action():
    """Test action preference for unknown action type."""
    alignment = MoralAlignment.LAWFUL_GOOD
    
    # Unknown action should return default neutral
    pref = AlignmentInfluence.get_action_preference(alignment, "unknown_action")
    assert pref == 0.5


# ============================================================================
# DIALOGUE TONE TESTS
# ============================================================================

@pytest.mark.unit
def test_dialogue_tone_lawful_good():
    """Test dialogue tone for Lawful Good alignment."""
    alignment = MoralAlignment.LAWFUL_GOOD
    tones = AlignmentInfluence.get_dialogue_tone(alignment)
    
    # Should be formal, friendly, helpful, respectful
    assert tones["formal"] > 0.7
    assert tones["friendly"] > 0.7
    assert tones["helpful"] > 0.7
    assert tones["respectful"] > 0.7
    
    # Should not be hostile or dismissive
    assert tones["hostile"] < 0.3
    assert tones["dismissive"] < 0.3


@pytest.mark.unit
def test_dialogue_tone_chaotic_neutral():
    """Test dialogue tone for Chaotic Neutral alignment."""
    alignment = MoralAlignment.CHAOTIC_NEUTRAL
    tones = AlignmentInfluence.get_dialogue_tone(alignment)
    
    # Should be casual and rebellious
    assert tones["casual"] > 0.7
    assert tones["rebellious"] > 0.7
    
    # Should not be formal
    assert tones["formal"] < 0.3


@pytest.mark.unit
def test_dialogue_tone_neutral_evil():
    """Test dialogue tone for Neutral Evil alignment."""
    alignment = MoralAlignment.NEUTRAL_EVIL
    tones = AlignmentInfluence.get_dialogue_tone(alignment)
    
    # Should be hostile and dismissive
    assert tones["hostile"] > 0.7
    assert tones["dismissive"] > 0.7
    
    # Should not be friendly or helpful
    assert tones["friendly"] < 0.3
    assert tones["helpful"] < 0.3


# ============================================================================
# COMPATIBILITY TESTS
# ============================================================================

@pytest.mark.unit
def test_compatibility_identical_alignments():
    """Test compatibility between identical alignments."""
    compat = AlignmentInfluence.calculate_compatibility(
        MoralAlignment.LAWFUL_GOOD,
        MoralAlignment.LAWFUL_GOOD
    )
    
    # Should be perfect compatibility
    assert compat == 1.0


@pytest.mark.unit
def test_compatibility_opposite_alignments():
    """Test compatibility between opposite alignments."""
    compat = AlignmentInfluence.calculate_compatibility(
        MoralAlignment.LAWFUL_GOOD,
        MoralAlignment.CHAOTIC_EVIL
    )
    
    # Should be minimum compatibility
    assert compat < 0.2


@pytest.mark.unit
def test_compatibility_adjacent_alignments():
    """Test compatibility between adjacent alignments."""
    compat = AlignmentInfluence.calculate_compatibility(
        MoralAlignment.LAWFUL_GOOD,
        MoralAlignment.NEUTRAL_GOOD
    )
    
    # Should have good compatibility (share good axis)
    assert compat > 0.7


@pytest.mark.unit
def test_compatibility_symmetric():
    """Test that compatibility is symmetric."""
    compat1 = AlignmentInfluence.calculate_compatibility(
        MoralAlignment.LAWFUL_GOOD,
        MoralAlignment.CHAOTIC_NEUTRAL
    )
    compat2 = AlignmentInfluence.calculate_compatibility(
        MoralAlignment.CHAOTIC_NEUTRAL,
        MoralAlignment.LAWFUL_GOOD
    )
    
    assert compat1 == compat2


# ============================================================================
# ACTION MORALITY TESTS
# ============================================================================

@pytest.mark.unit
def test_would_perform_action_aligned():
    """Test that NPCs perform actions aligned with their morality."""
    alignment = MoralAlignment.LAWFUL_GOOD
    
    # Lawful Good action (help someone, follow law)
    action_morality = (1.0, 1.0)  # Lawful and Good
    
    result = AlignmentInfluence.would_perform_action(
        alignment,
        action_morality,
        threshold=0.3
    )
    
    assert result is True


@pytest.mark.unit
def test_would_perform_action_misaligned():
    """Test that NPCs refuse actions misaligned with their morality."""
    alignment = MoralAlignment.LAWFUL_GOOD
    
    # Chaotic Evil action
    action_morality = (-1.0, -1.0)  # Chaotic and Evil
    
    result = AlignmentInfluence.would_perform_action(
        alignment,
        action_morality,
        threshold=0.3
    )
    
    assert result is False


@pytest.mark.unit
def test_would_perform_action_threshold():
    """Test that threshold affects action acceptance."""
    alignment = MoralAlignment.LAWFUL_GOOD
    
    # Slightly misaligned action
    action_morality = (0.5, 0.5)  # Mildly lawful and good
    
    # Strict threshold - should reject
    strict = AlignmentInfluence.would_perform_action(
        alignment,
        action_morality,
        threshold=0.1
    )
    
    # Lenient threshold - should accept
    lenient = AlignmentInfluence.would_perform_action(
        alignment,
        action_morality,
        threshold=0.5
    )
    
    assert strict is False
    assert lenient is True


@pytest.mark.unit
def test_would_perform_action_neutral_flexible():
    """Test that neutral alignments are more flexible."""
    alignment = MoralAlignment.TRUE_NEUTRAL
    
    # Various actions
    actions = [
        (1.0, 0.0),   # Lawful neutral
        (-1.0, 0.0),  # Chaotic neutral
        (0.0, 1.0),   # Neutral good
        (0.0, -1.0),  # Neutral evil
    ]
    
    # Neutral should accept more varied actions
    accepted = sum(
        AlignmentInfluence.would_perform_action(
            alignment,
            action,
            threshold=0.5
        )
        for action in actions
    )
    
    assert accepted >= 3  # Should accept most actions


# ============================================================================
# PARAMETRIZED TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("alignment,expected_law,expected_good", [
    (MoralAlignment.LAWFUL_GOOD, 1.0, 1.0),
    (MoralAlignment.NEUTRAL_GOOD, 0.0, 1.0),
    (MoralAlignment.CHAOTIC_GOOD, -1.0, 1.0),
    (MoralAlignment.LAWFUL_NEUTRAL, 1.0, 0.0),
    (MoralAlignment.TRUE_NEUTRAL, 0.0, 0.0),
    (MoralAlignment.CHAOTIC_NEUTRAL, -1.0, 0.0),
    (MoralAlignment.LAWFUL_EVIL, 1.0, -1.0),
    (MoralAlignment.NEUTRAL_EVIL, 0.0, -1.0),
    (MoralAlignment.CHAOTIC_EVIL, -1.0, -1.0),
])
def test_all_alignment_vectors(alignment, expected_law, expected_good):
    """Test vector values for all alignments."""
    vec = AlignmentInfluence.get_vector(alignment)
    assert vec.law_chaos == expected_law
    assert vec.good_evil == expected_good


@pytest.mark.unit
@pytest.mark.parametrize("alignment1,alignment2,expected_high_compat", [
    (MoralAlignment.LAWFUL_GOOD, MoralAlignment.LAWFUL_GOOD, True),
    (MoralAlignment.LAWFUL_GOOD, MoralAlignment.NEUTRAL_GOOD, True),
    (MoralAlignment.LAWFUL_GOOD, MoralAlignment.CHAOTIC_EVIL, False),
    (MoralAlignment.TRUE_NEUTRAL, MoralAlignment.LAWFUL_NEUTRAL, True),
])
def test_compatibility_patterns(alignment1, alignment2, expected_high_compat):
    """Test compatibility patterns between alignments."""
    compat = AlignmentInfluence.calculate_compatibility(alignment1, alignment2)
    
    if expected_high_compat:
        assert compat > 0.6
    else:
        assert compat < 0.4


@pytest.mark.unit
@pytest.mark.parametrize("alignment,good_action,should_prefer", [
    (MoralAlignment.LAWFUL_GOOD, "help_others", True),
    (MoralAlignment.LAWFUL_GOOD, "harm_others", False),
    (MoralAlignment.CHAOTIC_EVIL, "help_others", False),
    (MoralAlignment.CHAOTIC_EVIL, "break_rules", True),
    (MoralAlignment.LAWFUL_NEUTRAL, "follow_rules", True),
    (MoralAlignment.CHAOTIC_NEUTRAL, "embrace_chaos", True),
])
def test_action_preference_patterns(alignment, good_action, should_prefer):
    """Test action preference patterns across alignments."""
    pref = AlignmentInfluence.get_action_preference(alignment, good_action)
    
    if should_prefer:
        assert pref > 0.6
    else:
        assert pref < 0.4


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_alignment_vector_extreme_values():
    """Test alignment vector with extreme values."""
    vec = AlignmentVector(law_chaos=999.0, good_evil=-999.0)
    
    # Should be clamped to valid range
    assert vec.law_chaos == 1.0
    assert vec.good_evil == -1.0


@pytest.mark.unit
def test_compatibility_bounds():
    """Test that compatibility is always between 0 and 1."""
    alignments = list(MoralAlignment)
    
    for a1 in alignments:
        for a2 in alignments:
            compat = AlignmentInfluence.calculate_compatibility(a1, a2)
            assert 0.0 <= compat <= 1.0


@pytest.mark.unit
def test_action_preference_bounds():
    """Test that action preferences are always between 0 and 1."""
    alignments = list(MoralAlignment)
    actions = ["help_others", "harm_others", "follow_rules", "break_rules"]
    
    for alignment in alignments:
        for action in actions:
            pref = AlignmentInfluence.get_action_preference(alignment, action)
            assert 0.0 <= pref <= 1.0


@pytest.mark.unit
def test_dialogue_tone_bounds():
    """Test that dialogue tones are always between 0 and 1."""
    alignments = list(MoralAlignment)
    
    for alignment in alignments:
        tones = AlignmentInfluence.get_dialogue_tone(alignment)
        for tone_value in tones.values():
            assert 0.0 <= tone_value <= 1.0