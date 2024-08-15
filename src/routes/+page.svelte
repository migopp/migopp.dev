<script lang="ts">
	import { onMount, onDestroy } from 'svelte';

	// loading animation
	const cmd_text: string = 'cat README.txt';
	let typed_cmd_text: string = '_';
	let index: number = 0;
	let readme_visible: boolean = false;
	let prompt_visible: boolean = false;
	function sleep(ms: number): Promise<void> {
		return new Promise((resolve) => setTimeout(resolve, ms));
	}
	async function animation() {
		// initial delay
		await sleep(500);

		// typing text
		while (index < cmd_text.length) {
			// wait longer for periods before appear
			if (cmd_text[index] === '.') {
				await sleep(300);
			}

			typed_cmd_text = typed_cmd_text.slice(0, -1);
			typed_cmd_text += cmd_text[index];
			typed_cmd_text += '_';

			// wait longer for spaces and periods after appear
			let next: string = cmd_text[index];
			if (next === ' ' || next === '.') {
				await sleep(300);
			} else {
				await sleep(Math.random() * (140 - 100) + 100);
			}

			index++;
		}
		typed_cmd_text = typed_cmd_text.slice(0, -1);

		// README.txt contents appearance
		await sleep(1000);
		readme_visible = true;

		// prompt
		await sleep(200);
		prompt_visible = true;
	}
	onMount(() => animation());

	// blinking cursor animation
	let cursor_is_visible: boolean = true;
	const toggle_cursor = () => {
		cursor_is_visible = !cursor_is_visible;
	};
	const interval = setInterval(toggle_cursor, 500);
	onDestroy(() => clearInterval(interval));
</script>

<!--TERMINAL-->
<div id="terminal">
	<span id="prompt">
		<p id="host">anon@migopp.dev</p>
		<p id="buck">$</p>
		<p>{typed_cmd_text}</p>
	</span>
	<div id="readme" class={readme_visible ? 'visible' : 'invisible'}>
		<p>Howdy, and welcome to my corner of the internet.</p>
		<br />
		<p>
			I'm a developer and student at UT Austin studying Computer Science and Mathematics. Right now,
			I'm most interested in systems programming.
		</p>
		<br />
		<p>I'm also into cats and bagels.</p>
	</div>
	<span id="prompt" class={prompt_visible ? 'visible' : 'invisible'}>
		<p id="host">anon@migopp.dev</p>
		<p id="buck">$</p>
		<p>{cursor_is_visible ? '_' : ' '}</p>
	</span>
</div>

<style>
	:root {
		--t-grey: #3c3836;
		--t-red: #cc241d;
		--t-yellow: #d79921;
		--t-blue: #458588;
		--t-green: #8ec07c;
	}

	#terminal {
		height: 90%;
		width: 95%;
		min-height: 14rem;
		border-radius: 10px;
		padding: 0 0.5rem 1rem 0.75rem;
		background-color: var(--t-grey);
		font-family: 'Fira Code', monospace;
		color: white;
	}

	#prompt {
		display: flex;
		flex-direction: row;
		justify-content: flex-start;
		padding: 1rem 0 0.5rem 1rem;
	}

	#terminal #host {
		color: var(--t-green);
	}

	#terminal #buck {
		padding: 0 0.75rem 0 0;
	}

	#terminal #readme {
		padding: 0 0 0 1rem;
	}

	.visible {
		opacity: 1;
	}

	.invisible {
		opacity: 0;
	}

	* {
		margin: 0;
		padding: 0;
	}
</style>
