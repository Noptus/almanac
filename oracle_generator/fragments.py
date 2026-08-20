"""The fragment bank -- the actual product.

Short, warm, lightly literary message fragments tagged by astronomical
condition and tone. The voice is a cross between a stoic quote and a kind
mentor: reflective, reassuring, never mystical-scary, never a fortune-cookie
prediction. Nothing here claims to know the future.

Conventions
-----------
* Fragments are 1-2 sentences.
* `{name}` is OPTIONAL. The composition engine inserts it into at most one
  fragment per message, so every fragment must also read cleanly if `{name}`
  is stripped (the engine handles a dangling comma/space when it removes it).
* Each fragment is a (text, tone) tuple so the engine can balance tone.

Tones: reassuring, encouraging, cautionary_gentle, reflective, celebratory.
"""

from __future__ import annotations

# Type alias for readability: a fragment is (template_text, tone).
Fragment = tuple

TONES = ("reassuring", "encouraging", "cautionary_gentle", "reflective", "celebratory")


# ==========================================================================
# MOON PHASE FRAGMENTS  (8 phases, >=15 each)
# ==========================================================================

MOON_PHASE = {
    "new": [
        ("The sky holds no moon tonight, {name} -- a good night to plant an idea rather than defend one.", "encouraging"),
        ("New moon. Nothing is owed yet. Begin quietly.", "reassuring"),
        ("Everything starts unlit. Let this beginning stay small enough to keep.", "reflective"),
        ("A dark sky is not an empty one; it is a page before the first word.", "reflective"),
        ("Tonight the moon steps back so you can hear your own intention, {name}.", "encouraging"),
        ("Plant the seed, then cover it. Not everything worthwhile needs an audience yet.", "reassuring"),
        ("The new moon asks a soft question: what would you begin if no one were watching?", "reflective"),
        ("Beginnings rarely announce themselves. This one is content to be quiet.", "reassuring"),
        ("Under a moonless sky, small resolves count as much as grand ones.", "encouraging"),
        ("Nothing has to be finished today. It only has to be started.", "encouraging"),
        ("The lamp is unlit, {name}, but your hand still knows where the wick is.", "reassuring"),
        ("A fresh cycle opens. Spend it on one thing you actually want.", "encouraging"),
        ("Darkness at the start is ordinary, not ominous. Keep walking.", "reassuring"),
        ("First days are for intentions, not results. Set one and let it rest.", "reflective"),
        ("The moon is gathering itself again. So, quietly, are you.", "reassuring"),
        ("An empty sky is generous: it gives you room to imagine before you commit.", "reflective"),
    ],
    "waxing_crescent": [
        ("A thin blade of light returns. Small progress is still progress, {name}.", "encouraging"),
        ("The crescent leans forward like something learning to grow.", "reflective"),
        ("A little light is back in the sky. Add a little of your own.", "encouraging"),
        ("The moon is barely a sliver, yet it is unmistakably on its way.", "reassuring"),
        ("Momentum starts as a whisper. Listen for yours today.", "encouraging"),
        ("What you tended in the dark is beginning to show. Keep tending it.", "reassuring"),
        ("A crescent moon is proof that returning counts as arriving.", "reflective"),
        ("Growth this early is fragile and real at once. Protect it, {name}.", "reassuring"),
        ("The sky is filling in slowly. There is no rush written anywhere in it.", "reassuring"),
        ("One more line of light than yesterday. That is the whole task.", "encouraging"),
        ("Early effort feels like almost nothing. It is not nothing.", "encouraging"),
        ("The crescent is a promise the moon keeps every month. Keep yours.", "reflective"),
        ("Build on the small thing that worked. Ignore, for now, the large thing that didn't.", "encouraging"),
        ("A young moon doesn't apologise for being incomplete. Neither should you.", "reassuring"),
        ("The light is thin but climbing. Let today be a step, not a summit.", "reassuring"),
    ],
    "first_quarter": [
        ("Half-lit and climbing: the moon is at the point where effort meets resistance, {name}.", "encouraging"),
        ("First quarter. Half in shadow, half in light -- an honest picture of most decisions.", "reflective"),
        ("The moon has reached its first crossroads. So, perhaps, have you.", "reflective"),
        ("This is the phase of push. If something needs your weight against it, lean now.", "encouraging"),
        ("Half the sky is lit. That is enough to see the next step by.", "reassuring"),
        ("Tension between what you started and what it's asking of you is normal here.", "reassuring"),
        ("The moon does not pause at the halfway line, and neither must you, {name}.", "encouraging"),
        ("A quarter moon is a decision made visible: commit, or gently let go.", "reflective"),
        ("Resistance now is a sign you've reached the part that matters.", "encouraging"),
        ("Halfway is not stuck. Halfway is moving.", "reassuring"),
        ("The bright half and the dark half share the same moon. Let them share you too.", "reflective"),
        ("Push where it gives. Rest where it doesn't. The moon does both in turn.", "reassuring"),
        ("First-quarter light is the light of doing, not dreaming. Do a little.", "encouraging"),
        ("You are far enough in to feel the friction. That is the good news.", "encouraging"),
        ("Choose the harder half today, {name}, and the moon will meet you there.", "encouraging"),
    ],
    "waxing_gibbous": [
        ("Almost full. The moon is nearly there, and so, more than you think, are you, {name}.", "encouraging"),
        ("Waxing gibbous: the phase of refinement, not invention. Polish what you have.", "reflective"),
        ("The moon is fattening toward fullness. Trust the work you've already done.", "reassuring"),
        ("Nearly complete is a fine place to be. Resist the urge to start over.", "reassuring"),
        ("The last stretch before full is where patience earns its keep.", "encouraging"),
        ("Most of the sky is bright now. Attend to the small, dim corners that remain.", "reflective"),
        ("You are closer than the doubt in your head suggests, {name}.", "reassuring"),
        ("Gibbous light is generous. Let some of it fall on what you've made.", "celebratory"),
        ("Refinement is quieter than creation and just as important.", "reflective"),
        ("The moon rounds itself out slowly. Give your own edges the same grace.", "reassuring"),
        ("Nearly full means nearly ready. Steady hands from here.", "encouraging"),
        ("Don't confuse 'not finished' with 'not working.' It is working.", "reassuring"),
        ("The final degrees of a climb feel the steepest. Keep the pace kind.", "encouraging"),
        ("Almost-round, almost-there: the moon models the last mile beautifully.", "reflective"),
        ("Tend the details now, {name}; the shape is already good.", "encouraging"),
    ],
    "full": [
        ("The moon is full and unhidden tonight, {name}. Let something be seen that you usually keep quiet.", "celebratory"),
        ("Full moon. Whatever you've been building is standing in plain light now.", "celebratory"),
        ("Fullness is not the end of a thing, only its most visible moment.", "reflective"),
        ("The whole disc is lit. Pause long enough to notice how far the cycle has come.", "reflective"),
        ("Tonight the sky is at its brightest. Let yourself be, briefly, satisfied.", "celebratory"),
        ("A full moon reveals; it does not judge. Look kindly at what it shows you.", "reassuring"),
        ("Everything is illuminated tonight, {name} -- the good work and the loose ends alike.", "reflective"),
        ("Completion has its own light. Stand in it for a moment before the next thing.", "celebratory"),
        ("The moon holds nothing back tonight. Consider what you might stop hiding.", "encouraging"),
        ("Full light can feel like exposure. It is also, quietly, an invitation to be honest.", "reflective"),
        ("This is the summit of the cycle. Enjoy the view; the descent is gentle.", "celebratory"),
        ("What ripened this month is ripe now. Harvest a little of it, {name}.", "celebratory"),
        ("A full moon asks nothing new of you. It only asks you to notice.", "reassuring"),
        ("Brightness like this is temporary and worth marking. Mark it.", "celebratory"),
        ("The moon is whole tonight. Let one whole feeling have its turn.", "reflective"),
    ],
    "waning_gibbous": [
        ("The moon has begun to give its light back. So can you, {name} -- share what you learned.", "reflective"),
        ("Waning gibbous: the phase of gratitude and telling. Speak what worked.", "encouraging"),
        ("Just past full, the sky starts to soften. Softening is not the same as losing.", "reassuring"),
        ("The moon is spending down its brightness. Spend some of yours on others.", "encouraging"),
        ("This is the season of afterthought and thank-you. Both are worth your time.", "reflective"),
        ("Light is receding, gently. There is nothing to fix in that; it is the design.", "reassuring"),
        ("What you gathered at the full, {name}, is yours to pass along now.", "encouraging"),
        ("The disc is dimming a little each night. Let the day feel a little less urgent too.", "reassuring"),
        ("After the peak comes the sharing. Generosity suits this phase.", "encouraging"),
        ("The moon teaches how to let go without alarm. Take the lesson slowly.", "reflective"),
        ("Reflection is the natural work of a waning sky. Do a little tonight.", "reflective"),
        ("You don't have to hold the whole light. Set some of it down, {name}.", "reassuring"),
        ("The brightest part is behind you, and that is perfectly, usefully fine.", "reassuring"),
        ("Tell someone what the last cycle taught you. That is how light gets shared.", "encouraging"),
        ("A dimming moon is still a moon. You, dimming a little, are still you.", "reassuring"),
    ],
    "last_quarter": [
        ("Half-dark and falling: the moon is clearing space now. Consider what to release, {name}.", "reflective"),
        ("Last quarter. Half the light is already gone, and the sky is not distressed about it.", "reassuring"),
        ("This is the phase of letting go on purpose rather than by accident.", "reflective"),
        ("The waning half-moon marks a turn inward. Follow it a little way.", "reflective"),
        ("Clearing out is honest work. Do some of it while the moon models the way.", "encouraging"),
        ("Half is lit, half is done with being lit. Some things you can put down now.", "reassuring"),
        ("The moon is subtracting, calmly. Subtraction can be a kindness, {name}.", "reassuring"),
        ("What no longer fits can be set at the edge of the path and left there.", "encouraging"),
        ("A last-quarter sky asks a fair question: what has finished its work in you?", "reflective"),
        ("Release is not defeat. The moon proves it every month without complaint.", "reassuring"),
        ("Make a little room. The next cycle will want it.", "encouraging"),
        ("The descending half-moon is a good hour for forgiveness -- including your own.", "reassuring"),
        ("Loosen your grip on one thing today, {name}. Just one is enough.", "encouraging"),
        ("Half-light is enough light to sort by. Keep what serves; release what strains.", "reflective"),
        ("The sky is emptying toward rest. Let part of your day empty too.", "reassuring"),
    ],
    "waning_crescent": [
        ("Only a thin arc remains, {name}. The cycle is nearly closed; let yourself rest before the next.", "reassuring"),
        ("Waning crescent: the last light before the dark. A phase for rest, not resolve.", "reassuring"),
        ("The moon is a fading curve now, unhurried about its own ending.", "reflective"),
        ("Almost gone is not the same as gone. There is still a little light to steer by.", "reassuring"),
        ("This is the sky's quiet exhale. Match it if you can.", "reflective"),
        ("The old moon thins toward nothing so a new one can begin. Trust the turn.", "reassuring"),
        ("Little is left of this cycle's light, {name}, and that is exactly on schedule.", "reassuring"),
        ("Rest is not idleness here; it is preparation. Let it be enough.", "reassuring"),
        ("The final sliver asks nothing of you but stillness. Grant it.", "reassuring"),
        ("Endings, like this crescent, can be gentle. Let this one be.", "reflective"),
        ("Close the cycle kindly. Loose ends can wait for the new moon.", "encouraging"),
        ("The last curve of light is a good place to be quiet and grateful.", "reflective"),
        ("You have carried enough for one cycle. Set it down before dawn, {name}.", "reassuring"),
        ("A vanishing moon teaches that emptying makes room. Believe it a little.", "reflective"),
        ("Soon the sky resets. Until then, be soft with yourself.", "reassuring"),
    ],
}


# ==========================================================================
# PLANETARY EMPHASIS FRAGMENTS  (5 planets x direct/retrograde = 10, >=10 each)
# ==========================================================================

PLANETARY_EMPHASIS = {
    "mercury_retrograde": [
        ("Mercury moves backward through the sky today. Reread before you send, {name}.", "cautionary_gentle"),
        ("With Mercury retrograde, slow speech is wise speech. Say the second thing, not the first.", "cautionary_gentle"),
        ("Messages tangle more easily now. Ask 'did you mean...?' before you assume.", "cautionary_gentle"),
        ("Mercury retrograde is a fine excuse to finish an old draft rather than start a new one.", "encouraging"),
        ("Double-check the small print today; the sky is in a backtracking mood.", "cautionary_gentle"),
        ("When Mercury reverses, revisiting beats rushing. Return to something unfinished.", "reflective"),
        ("A misheard word costs little if you're willing to ask again. Ask again, {name}.", "cautionary_gentle"),
        ("Mercury retrograde favours the patient editor over the quick sender.", "reflective"),
        ("Travel and timing wobble a bit now. Leave a margin and let the day breathe.", "cautionary_gentle"),
        ("This is a good week to repair a conversation, not to force a new one.", "encouraging"),
        ("The sky suggests re-, not new-: review, repair, reconnect.", "reflective"),
        ("If a plan feels slippery today, it may just be Mercury. Firm your grip gently.", "reassuring"),
    ],
    "mercury_direct": [
        ("Mercury runs forward and clear today. If there's a thing to say, {name}, say it plainly.", "encouraging"),
        ("The channels are open. Send the note you've been holding.", "encouraging"),
        ("A good day for straight talk and clean plans. Use the clarity.", "encouraging"),
        ("Mercury direct favours the messenger. Make the call you've been circling.", "encouraging"),
        ("Words land where you aim them today. Aim kindly.", "reflective"),
        ("Thinking feels less tangled now. Untangle one thing while it's easy.", "encouraging"),
        ("The mind's roads are open, {name}. Take the direct route for once.", "encouraging"),
        ("Clarity is on offer today. Spend it on something that's been vague too long.", "encouraging"),
        ("A clear-signal day: write it down, say it out loud, make it real.", "encouraging"),
        ("Mercury moves ahead cleanly. So can the conversation you've postponed.", "encouraging"),
        ("Ideas travel well today. Let one leave your head and meet the world.", "encouraging"),
    ],
    "venus_retrograde": [
        ("Venus turns back today, {name}. It's a season to revisit what you value, not to spend.", "reflective"),
        ("With Venus retrograde, old feelings may resurface. Meet them with curiosity, not alarm.", "reassuring"),
        ("The sky asks you to review your affections rather than rearrange them. Look before you leap.", "cautionary_gentle"),
        ("Venus retrograde is kinder to reflection than to purchase. Pause the cart.", "cautionary_gentle"),
        ("Beauty turns inward now. Tend what you already love before seeking more.", "reflective"),
        ("A returning Venus reopens old chapters. You may reread; you needn't rewrite.", "reflective"),
        ("Value what's quietly loyal today over what's newly shiny, {name}.", "reflective"),
        ("This is a week for gratitude toward what you have, not longing for what you don't.", "reassuring"),
        ("If an old fondness resurfaces, let it inform you before it moves you.", "cautionary_gentle"),
        ("Venus retrograde favours mending over acquiring. Mend a small thing.", "encouraging"),
        ("Hold big commitments loosely this week; the sky is still deciding with you.", "cautionary_gentle"),
    ],
    "venus_direct": [
        ("Venus moves forward and warm today. Let a little beauty in on purpose, {name}.", "celebratory"),
        ("A generous sky for affection and craft. Make something, or tell someone.", "encouraging"),
        ("Venus direct favours the open hand. Offer, share, appreciate.", "celebratory"),
        ("Harmony is a little easier to reach today. Reach for it first.", "encouraging"),
        ("The day leans toward pleasure that's honest. Choose one small good thing.", "celebratory"),
        ("Venus runs clear now. Say the kind thing before the moment passes.", "encouraging"),
        ("Beauty is uncomplicated today, {name}. Notice some of it and let that be enough.", "reflective"),
        ("A fine day to repair goodwill or simply enjoy it. Both count.", "encouraging"),
        ("Value moves outward now. Give a little of what you'd like to receive.", "encouraging"),
        ("Warmth travels well under a direct Venus. Send some.", "celebratory"),
        ("Let yourself like what you like today, without a defence prepared.", "reassuring"),
    ],
    "mars_retrograde": [
        ("Mars turns backward today, {name}. Conserve your fire; not every provocation deserves it.", "cautionary_gentle"),
        ("With Mars retrograde, the wise move is often the unhurried one. Let the anger cool.", "cautionary_gentle"),
        ("Energy pulls inward now. Train, don't fight; plan, don't charge.", "encouraging"),
        ("A retrograde Mars rewards patience over force. Wait for the better angle.", "reflective"),
        ("Push less today and aim more. The target isn't going anywhere.", "cautionary_gentle"),
        ("Frustration is loud under this sky. You don't have to answer it out loud, {name}.", "reassuring"),
        ("Mars retrograde is a good time to sharpen the tool, not swing it.", "reflective"),
        ("Redirect the heat inward as resolve rather than outward as reaction.", "encouraging"),
        ("If a fight offers itself today, you're allowed to decline the invitation.", "cautionary_gentle"),
        ("Slow force is still force. Move deliberately and you lose nothing.", "reassuring"),
        ("The sky counsels restraint over conquest this week. Rest the sword arm.", "cautionary_gentle"),
    ],
    "mars_direct": [
        ("Mars drives forward and clean today, {name}. If action's been waiting, this is its hour.", "encouraging"),
        ("The sky backs the doers now. Take the first concrete step.", "encouraging"),
        ("Mars direct favours momentum. Point your energy at one clear thing.", "encouraging"),
        ("Courage is a little cheaper today. Spend some on the task you've dodged.", "encouraging"),
        ("A day for honest effort. Begin, and let beginning be the win.", "encouraging"),
        ("Mars runs ahead cleanly. So can the project you've kept idling.", "encouraging"),
        ("Vigour is on offer, {name}. Aim it, don't just feel it.", "encouraging"),
        ("Directness suits the sky now. Do the plain, brave, boring next step.", "encouraging"),
        ("The path forward is unobstructed today. Walk a stretch of it.", "encouraging"),
        ("Strength is easiest to find in motion. Move first, feel ready after.", "encouraging"),
        ("Let today's energy build something rather than burn something.", "reflective"),
    ],
    "jupiter_retrograde": [
        ("Jupiter turns inward today, {name}. Growth this season is depth, not distance.", "reflective"),
        ("With Jupiter retrograde, the expansion is internal. Grow roots before branches.", "reflective"),
        ("The sky favours reviewing your beliefs over broadcasting them now.", "cautionary_gentle"),
        ("A retrograde Jupiter asks: is the goal still yours, or just old momentum?", "reflective"),
        ("Luck turns philosophical this week. Learn from a delay instead of resenting it.", "reassuring"),
        ("Expand quietly, {name}. The best growth right now doesn't need a crowd.", "reflective"),
        ("Jupiter retrograde rewards the honest audit over the bold gamble.", "cautionary_gentle"),
        ("Reconsider a big promise before you enlarge it. There's time.", "cautionary_gentle"),
        ("Wisdom compounds inwardly now. Sit with a question longer than usual.", "reflective"),
        ("The sky counsels meaning over more. Ask what a thing is for.", "reflective"),
        ("Optimism sharpens when it's examined. Examine yours kindly.", "reflective"),
    ],
    "jupiter_direct": [
        ("Jupiter moves forward and open today, {name}. Room is being made; step into a little of it.", "encouraging"),
        ("The sky favours growth now. Say yes to one thing slightly larger than yesterday.", "encouraging"),
        ("Jupiter direct widens the road. Take the more generous view.", "encouraging"),
        ("Opportunity travels well today. Keep your yes ready and your reasons kind.", "encouraging"),
        ("A day that rewards a bit of faith. Extend some to your own plans.", "reassuring"),
        ("Expansion is easier under a direct Jupiter. Reach one notch further.", "encouraging"),
        ("Good fortune likes motion, {name}. Meet it partway.", "encouraging"),
        ("The horizon looks a little closer today. Walk toward it without hurry.", "encouraging"),
        ("Generosity multiplies now. Be the one who offers first.", "celebratory"),
        ("Let the day's optimism be practical: one open door, one honest step.", "encouraging"),
        ("Growth is on offer. Choose the kind that also lets others grow.", "reflective"),
    ],
    "saturn_retrograde": [
        ("Saturn turns back today, {name}. Revisit the structure before you build higher on it.", "reflective"),
        ("With Saturn retrograde, the sky rewards repair of foundations over new floors.", "cautionary_gentle"),
        ("Discipline turns reflective now. Ask which rules still serve you.", "reflective"),
        ("A retrograde Saturn is a chance to renegotiate a burden, not just carry it.", "reassuring"),
        ("Review your commitments this week. Some can be lightened without being broken.", "reassuring"),
        ("The sky favours consolidation over construction. Firm up what wobbles.", "cautionary_gentle"),
        ("Responsibility feels heavier under this sky, {name}. Set part of it down to rest.", "reassuring"),
        ("Saturn retrograde asks patience with slow things. Yours included.", "reflective"),
        ("Rework the plan's frame before you decorate its rooms.", "cautionary_gentle"),
        ("Old duties resurface now. Honour the ones that still fit; retire the rest.", "reflective"),
        ("Structure is worth revisiting, not worshipping. Adjust one beam.", "reflective"),
    ],
    "saturn_direct": [
        ("Saturn moves forward and steady today, {name}. Lay one solid stone and trust it to hold.", "encouraging"),
        ("The sky rewards patient structure now. Do the unglamorous, lasting thing.", "encouraging"),
        ("Saturn direct favours the builder. Commit to a small, real discipline.", "encouraging"),
        ("Foundations set well today. Lay one and let it settle.", "encouraging"),
        ("A day for keeping a promise to yourself. Keep a plain one.", "reassuring"),
        ("Saturn steadies the ground now. Stand on it and plan the next course.", "encouraging"),
        ("Diligence pays quietly today, {name}. Choose the durable over the flashy.", "reflective"),
        ("Structure is a kindness to your future self. Build a little of it now.", "reflective"),
        ("The sky backs the reliable today. Be reliable to one thing that matters.", "encouraging"),
        ("Slow and solid wins under a direct Saturn. Set the pace and hold it.", "encouraging"),
        ("Order is on offer. Put one corner of your life gently in its place.", "encouraging"),
    ],
}


# ==========================================================================
# SEASON MARKER FRAGMENTS
# ==========================================================================

SEASON_MARKER = {
    "solstice": [
        ("The sun stands still at its extreme today, {name} -- the year pauses, and you may too.", "reflective"),
        ("Solstice. This is the hinge of the year; notice which way the light is about to turn.", "reflective"),
        ("At the solstice the day is longest or shortest -- either way, a threshold worth marking.", "celebratory"),
        ("The sun reaches the edge of its road and turns back. Turnings deserve a breath.", "reflective"),
        ("A solstice is the year holding its breath. Hold yours a moment with it.", "reflective"),
        ("Extremes don't last; the solstice proves it. From here, balance returns slowly.", "reassuring"),
        ("Mark this turning, {name}. The light has reached its limit and begins its long answer.", "celebratory"),
        ("The solstice is a natural new leaf. Write one intention on it and close the book gently.", "encouraging"),
    ],
    "equinox": [
        ("Day and night are equal today, {name}. Let something in your own life come level too.", "reflective"),
        ("Equinox. Light and dark share the sky exactly; balance is briefly, beautifully literal.", "reflective"),
        ("The equinox splits the day evenly. Consider where your own scales want adjusting.", "reflective"),
        ("Perfect balance is rare and doesn't stay. The equinox offers a taste; take it.", "reassuring"),
        ("Equal parts light and dark, {name} -- a fair reminder that both belong to a full day.", "reflective"),
        ("The equinox is the year's level ground. Stand on it and check your footing.", "reflective"),
        ("Balance is a moment, not a destination. The sky demonstrates it today.", "reflective"),
        ("At the equinox the world is even-handed. Try being even-handed with yourself.", "reassuring"),
    ],
    "ordinary": [
        ("No great turning in the sky today, {name} -- just an ordinary day, which is where most of a life is lived.", "reassuring"),
        ("The heavens are unremarkable today, and that is its own quiet gift.", "reassuring"),
        ("An ordinary day asks little and offers plenty if you let it.", "reflective"),
        ("Nothing is peaking overhead. Good -- steadiness is underrated.", "reassuring"),
        ("Most days are plain days. Plain days are where the real work happens.", "reflective"),
        ("The sky is calm today, {name}. Let that permission reach the rest of you.", "reassuring"),
        ("No cosmic drama tonight. Just you, the evening, and whatever you choose to make of it.", "reflective"),
        ("An unhurried sky suits an unhurried heart. Take the day as it comes.", "reassuring"),
    ],
}


# ==========================================================================
# CLOSING LINES  (>=20, tone-tagged)
# ==========================================================================

CLOSING_LINES = [
    ("Tomorrow's page is already turning. Sleep well, {name}.", "reassuring"),
    ("Whatever today held, the sky resets by morning. So can you.", "reassuring"),
    ("Take one small kindness into the evening -- ideally toward yourself.", "reassuring"),
    ("The stars keep their own schedule. You're allowed to keep yours.", "reassuring"),
    ("Rest is not a reward for finishing; it's part of the work. Take some.", "reassuring"),
    ("Let the day close without a verdict. Not everything needs judging tonight.", "reflective"),
    ("Carry forward only what serves you. Leave the rest to the dark.", "encouraging"),
    ("The moon will be here tomorrow, and so, quietly, will you.", "reassuring"),
    ("Be gentle in the last hour of the day. It sets the tone for the next.", "reassuring"),
    ("One honest breath is a fine way to end an ordinary evening.", "reflective"),
    ("You did enough today. Let that sentence be true.", "reassuring"),
    ("Close the day like a book you'll open again -- without tearing the page.", "reflective"),
    ("The sky asks nothing more of you tonight. Neither should you, {name}.", "reassuring"),
    ("Small steps still count after dark. So does stopping.", "encouraging"),
    ("Let tomorrow be tomorrow's. Tonight is for setting things down.", "reassuring"),
    ("Somewhere overhead the pattern continues, steady and unbothered. Borrow its calm.", "reflective"),
    ("Kindness scales. Offer a little to the person you'll be in the morning.", "encouraging"),
    ("The best plans can wait until you've slept. Sleep first.", "reassuring"),
    ("Mark one good thing from today before you forget it happened.", "celebratory"),
    ("The day is done and that is not a failure -- it's the design. Rest, {name}.", "reassuring"),
    ("Let the last thought of the day be a soft one.", "reassuring"),
    ("Tomorrow keeps its own light. Trust it to arrive.", "reassuring"),
]
