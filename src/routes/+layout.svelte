<script lang="ts">
	let current_year: number = new Date().getFullYear();
</script>

<svelte:head>
	<link
		href="https://fonts.googleapis.com/css2?family=Fira+Code:wght@300..700&family=Noto+Serif+JP:wght@200..900&family=Spectral:ital,wght@0,200;0,300;0,400;0,500;0,600;0,700;0,800;1,200;1,300;1,400;1,500;1,600;1,700;1,800&display=swap"
		rel="stylesheet"
	/>
</svelte:head>

<div id="layout-container">
	<div id="layout">
		<header>
			<a href="/" id="migopp">migopp 🌱</a>

			<nav>
				<ul>
					<li><a href="/things">things</a></li>
					<li>|</li>
					<li><a href="/thoughts">thoughts</a></li>
					<li>|</li>
					<li>track record</li>
				</ul>
			</nav>
		</header>

		<main>
			<slot></slot>
		</main>

		<footer>
			<ul>
				<li class="jp">僕の心から</li>
				<li style="padding-top: 0.25rem;">&copy; {current_year} Michael Goppert</li>
			</ul>
		</footer>
	</div>
</div>

<style>
	#layout-container {
		max-width: 800px;
		margin: 0 auto;
		font-family: 'Spectral', monospace;
	}

	#layout {
		display: flex;
		flex-direction: column;
		margin: 0 0.5rem;
	}

	#migopp {
		font-size: 1.75rem;
		font-weight: 700;
	}

	.jp {
		font-family: 'Noto Serif JP', serif;
		font-weight: 400;
	}

	* {
		margin: 0;
		padding: 0;
	}

	header,
	main {
		height: 100%;
	}

	main {
		flex: 1;
	}

	header,
	footer {
		margin: 1rem 0;
	}

	header ul,
	footer ul {
		list-style: none;
		display: flex;
		align-items: center;
	}
	header li,
	footer li {
		font-size: 1.1rem;
		margin-right: 1rem;
	}

	header a,
	footer a {
		text-decoration: none;
		color: inherit;
	}
</style>
