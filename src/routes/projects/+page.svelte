<script lang="ts">
	import projects from '$lib/projects.json';
</script>

<svelte:head>
	<title>projects</title>
</svelte:head>

<h1>projects</h1>
<p>A few of my projects.</p>
{#each projects as project}
	<div id="project">
		<span id="title-line">
			<h2 id="title">{project.name}</h2>
			{#if project.link}
				<a href={project.link} target="_blank" rel="noopener noreferrer">🔗</a>
			{/if}
		</span>
		<p>{project.description}</p>
		<p id="technologies">Technologies: {project.technologies}</p>
	</div>
{/each}

<style>
	#project {
		margin-bottom: 0.5rem;
	}

	#title-line {
		display: flex;
		flex-direction: row;
		justify-content: flex-start;
		align-items: center;
	}

	#title {
		padding-right: 0.5rem;
	}

	#technologies {
		font-style: italic;
	}

	* {
		margin: 0;
		padding: 0;
	}

	a {
		text-decoration: none;
		color: inherit;
		font-size: 1.2rem;
	}
</style>
